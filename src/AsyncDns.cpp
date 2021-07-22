//
// Created by v4kst1z
//
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
}

#include <cstring>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "../include/AsyncDns.h"
#include "../include/Common.h"
#include "../include/Ipv4Addr.h"
#include "../include/Looper.h"
#include "../include/UdpConnection.h"

AsyncDns::AsyncDns()
    : quit_(false),
      dns_socket_(sockets::CreateNonblockAndCloexecUdpSocket()),
      log_(Logger::GetInstance()),
      queue_domain_(new SafeQueue<DnsMessage>()),
      dns_server_addr_(new Ipv4Addr("8.8.8.8", 53)) {
  looper_ = new Looper<UdpConnection>(nullptr);
  looper_->SetLoopFlag(LOOPFLAG::UDPCLIENT);
  log_.Start();
}

AsyncDns::AsyncDns(Looper<UdpConnection> *looper)
    : quit_(false),
      dns_socket_(sockets::CreateNonblockAndCloexecUdpSocket()),
      looper_(looper),
      log_(Logger::GetInstance()),
      queue_domain_(new SafeQueue<DnsMessage>()),
      dns_server_addr_(new Ipv4Addr("8.8.8.8", 53)) {
  log_.Start();
}

AsyncDns::AsyncDns(Looper<UdpConnection> *looper,
                   std::shared_ptr<Ipv4Addr> dns_server_addr)
    : quit_(false),
      dns_socket_(sockets::CreateNonblockAndCloexecUdpSocket()),
      looper_(looper),
      log_(Logger::GetInstance()),
      queue_domain_(new SafeQueue<DnsMessage>()),
      dns_server_addr_(dns_server_addr) {
  log_.Start();
}

void AsyncDns::StartLoop() { dns_thread_ = std::thread(&AsyncDns::Loop, this); }

void AsyncDns::Loop() {
  looper_->Start();
  int error = 0;
  sockets::Connect(dns_socket_, dns_server_addr_.get(), error);
  EventBase<Event> fd = Event(dns_socket_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback([this]() {
    std::string domain;
    std::string ip = ParseResponse(domain);
    for (auto fd : domain_to_fd_[domain]) {
      write(fd, ip.data(), ip.size());  //唤醒 fd
    }
    domain_to_fd_.erase(domain);
    domain_to_ip_.insert({domain, ip});
  });
  looper_->AddEvent(std::make_shared<VariantEventBase>(fd));

  while (true) {
    if (quit_ && queue_domain_->Empty()) break;
    std::unique_ptr<DnsMessage> data = queue_domain_->WaitPop();
    domain_to_fd_[data->domain_].push_back(data->fd_);
    looper_->AddTask(std::bind(&AsyncDns::CreateDnsQuery, this, data->domain_,
                               data->domain_len_));
  }
}

void AsyncDns::AddDnsQuery(int fd, std::string &domain) {
  queue_domain_->Push(
      std::unique_ptr<DnsMessage>(new DnsMessage(fd, domain.size(), domain)));
}

void AsyncDns::AddDnsQuery(int fd, const char *domain) {
  queue_domain_->Push(
      std::unique_ptr<DnsMessage>(new DnsMessage(fd, strlen(domain), domain)));
}

std::string AsyncDns::GetIp(std::string domain) {
  if (domain_to_ip_.find(domain) != domain_to_ip_.end())
    return domain_to_ip_[domain];
  else
    return "";
}

DnsHeader *AsyncDns::CreateHeader() {
  DnsHeader *header = new DnsHeader;
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> ushort(0, 65535);
  auto num = ushort(generator);
  header->trans_id_ = ushort(generator);
  header->flags_ = htons(0x0100);
  header->qestions_num_ = htons(1);
  return header;
}

DnsQuestion *AsyncDns::CreateQuestion(std::string domain, int domain_len) {
  std::string delimiter = ".";
  size_t pos = 0;
  std::string res = "";
  size_t len = 0;

  DnsQuestion *ques = new DnsQuestion(domain_len + 2);
  ques->question_class_ = htons(1);
  ques->question_type_ = htons(1);
  memset(ques->domain_, '\x00', ques->domain_len_);

  while ((pos = domain.find(delimiter)) != std::string::npos) {
    ques->domain_[len++] = pos;
    memcpy(ques->domain_ + len, domain.data(), pos);
    len += pos;
    domain.erase(0, pos + 1);
  }
  ques->domain_[len++] = domain.size();
  memcpy(ques->domain_ + len, domain.data(), domain.size());
  return ques;
}

void AsyncDns::CreateDnsQuery(std::string domain, int len) {
  DnsHeader *header = CreateHeader();
  DnsQuestion *ques = CreateQuestion(domain, len);
  size_t ques_size = ques->domain_len_ + 4;
  Dns *dns_query = static_cast<Dns *>(malloc(sizeof(Dns) + ques_size));

  memcpy(dns_query, header, 12);
  memcpy((char *)(dns_query) + 12, ques->domain_, ques->domain_len_);
  memcpy((char *)dns_query + 12 + ques->domain_len_, &ques->question_type_, 4);

  sockets::SendTo(dns_socket_, dns_server_addr_.get(),
                  reinterpret_cast<const char *>(dns_query),
                  12 + ques->domain_len_ + 4);

  delete header;
  delete ques;
  delete dns_query;
  return;
}

std::string AsyncDns::ParseResponse(std::string &domain) {
  struct sockaddr_in addr;
  size_t addr_len = sizeof(struct sockaddr_in);

  char buffer[BUFSIZ];
  memset(buffer, '\x00', BUFSIZ);

  int n = recvfrom(dns_socket_, buffer, BUFSIZ, 0, (struct sockaddr *)&addr,
                   (socklen_t *)&addr_len);

  size_t idx = 0;
  // parse dns header
  std::unique_ptr<DnsHeader> header(new DnsHeader);
  header->trans_id_ = *((unsigned short *)buffer + idx++);
  header->flags_ = ntohs(*((unsigned short *)buffer + idx++));
  header->qestions_num_ = ntohs(*((unsigned short *)buffer + idx++));
  header->answer_num_ = ntohs(*((unsigned short *)buffer + idx++));
  header->aut_num_ = ntohs(*((unsigned short *)buffer + idx++));
  header->add_num_ = ntohs(*((unsigned short *)buffer + idx++));

  // parse dns header
  std::unique_ptr<DnsQuestion> ques(new DnsQuestion(strlen(buffer + 12) + 1));
  char domain_chars[128];
  // 3www5baidu3com
  memset(domain_chars, '\x00', 128);

  memcpy(domain_chars, buffer + 13, buffer[12]);
  int id = 0;
  for (id = buffer[12 + id] + 1; id < ques->domain_len_; id++) {
    domain_chars[strlen(domain_chars)] = '.';
    int len = buffer[12 + id];
    if (len == 0) break;
    memcpy(domain_chars + strlen(domain_chars), buffer + 12 + id + 1, len);
    id += len;
  }
  domain_chars[strlen(domain_chars) - 1] = '\x00';
  domain = domain_chars;
  memcpy(ques->domain_, buffer + 12, ques->domain_len_);
  ques->question_type_ =
      ntohs(*(unsigned short *)(buffer + 12 + ques->domain_len_));
  ques->question_class_ =
      ntohs(*(unsigned short *)(buffer + 14 + ques->domain_len_));

  // parse dns answer
  idx = 12 + ques->domain_len_ + 4;
  for (unsigned short id = 0; id < header->answer_num_; id++) {
    DnsAnswer ans;
    ans.name_ = ntohs(*(unsigned short *)(buffer + idx));
    ans.answer_type_ = ntohs(*(unsigned short *)(buffer + idx + 2));
    ans.answer_class_ = ntohs(*(unsigned short *)(buffer + idx + 4));
    ans.time_to_live_ = ntohl(*(unsigned int *)(buffer + idx + 6));
    ans.data_len_ = ntohs(*(unsigned short *)(buffer + idx + 10));

    if (ans.answer_type_ == 1) {
      char ip[INET_ADDRSTRLEN];
      sprintf(ip, "%s", buffer + idx + 12);
      inet_ntop(AF_INET, buffer + idx + 12, ip, INET_ADDRSTRLEN);
      ans.ip_ = ip;
      return ans.ip_;
    }
    idx += 12 + ans.data_len_;
  }
  return "";
}
