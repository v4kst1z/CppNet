//
// Created by v4kst1z
//
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
}

#include <random>
#include <vector>

#include "AsyncDns.h"
#include "Ipv4Addr.h"
#include "Looper.h"
#include "UdpConnection.h"

AsyncDns::AsyncDns(Looper<UdpConnection> *looper)
    : quit_(false),
      looper_(looper),
      log_(Logger::GetInstance()),
      dns_server_addr_(new Ipv4Addr("114.114.114.114", 53)) {
  looper->SetLoopFlag(LOOPFLAG::UDPCLIENT);
  log_.Start();
  CreateDnsQuery(make_unique<DnsMessage>(strlen("www.jd.com"), "www.jd.com"));
}

AsyncDns::AsyncDns(Looper<UdpConnection> *looper,
                   std::shared_ptr<Ipv4Addr> dns_server_addr)
    : quit_(false),
      looper_(looper),
      log_(Logger::GetInstance()),
      dns_server_addr_(dns_server_addr) {
  log_.Start();
}

void AsyncDns::SetNewConnCallback(UdpConnection::CallBack &&cb) {
  new_conn_callback_ = std::move(cb);
  looper_->SetNewConnCallback(new_conn_callback_);
}

void AsyncDns::SetMessageCallBack(UdpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
  looper_->SetMessageCallBack(message_callback_);
}

void AsyncDns::SetCloseCallBack(UdpConnection::CallBack &&cb) {
  close_callback_ = std::move(cb);
  looper_->SetCloseCallBack(close_callback_);
}

void AsyncDns::SetSendDataCallBack(UdpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
  looper_->SetSendDataCallBack(send_data_callback_);
}

void AsyncDns::SetErrorCallBack(UdpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
  looper_->SetErrorCallBack(error_callback_);
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

DnsQuestion *AsyncDns::CreateQuestion(std::unique_ptr<DnsMessage> domain) {
  std::string delimiter = ".";
  size_t pos = 0;
  std::string res = "";
  size_t len = 0;

  DnsQuestion *ques = new DnsQuestion(domain->domain_len_ + 2);
  ques->question_class_ = htons(1);
  ques->question_type_ = htons(1);
  memset(ques->domain_, '\x00', ques->domain_len_);

  while ((pos = domain->domain_.find(delimiter)) != std::string::npos) {
    ques->domain_[len++] = pos;
    memcpy(ques->domain_ + len, domain->domain_.data(), pos);
    len += pos;
    domain->domain_.erase(0, pos + 1);
  }
  ques->domain_[len++] = domain->domain_.size();
  memcpy(ques->domain_ + len, domain->domain_.data(), domain->domain_.size());
  return ques;
}

Dns *AsyncDns::CreateDnsQuery(std::unique_ptr<DnsMessage> domain) {
  DnsHeader *header = CreateHeader();
  DnsQuestion *ques = CreateQuestion(std::move(domain));
  size_t ques_size = ques->domain_len_ + 4;
  Dns *dns_query = static_cast<Dns *>(malloc(sizeof(Dns) + ques_size));

  memcpy(dns_query, header, 12);
  memcpy((char *)(dns_query) + 12, ques->domain_, ques->domain_len_);
  memcpy((char *)dns_query + 12 + ques->domain_len_, &ques->question_type_, 4);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  int slen = sendto(sock, dns_query, 12 + ques->domain_len_ + 4, 0,
                    (struct sockaddr *)dns_server_addr_->GetAddr(),
                    sizeof(struct sockaddr));
  struct sockaddr_in addr;
  size_t addr_len = sizeof(struct sockaddr_in);

  char buf[BUFSIZ];
  memset(buf, '\x00', BUFSIZ);

  int n = recvfrom(sock, buf, BUFSIZ, 0, (struct sockaddr *)&addr,
                   (socklen_t *)&addr_len);

  for (int id = 0; id < 1024; id++) {
    printf(" %x", buf[id] & 0xff);
  }
  std::cout << ParseResponse(buf) << std::endl;
  delete header;
  delete ques;
  return dns_query;
}

std::string AsyncDns::ParseResponse(char *buffer) {
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
