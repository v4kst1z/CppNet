//
// Created by v4kst1z
//

#ifndef CPPNET_NET_ASYNCDNS_H_
#define CPPNET_NET_ASYNCDNS_H_

#include <thread>
#include <unordered_map>

#include "SafeQueue.h"
#include "UdpConnection.h"

class Logger;
class UdpConnection;

template <typename T>
class Looper;

typedef struct DnsHeader {
 public:
  unsigned short trans_id_;
  unsigned short flags_;
  unsigned short qestions_num_;
  unsigned short answer_num_;
  unsigned short aut_num_;
  unsigned short add_num_;
} DnsHeader;

typedef struct DnsQuestion {
 public:
  explicit DnsQuestion(size_t len) : domain_len_(len) {
    domain_ = static_cast<char *>(malloc(len));
  }

  ~DnsQuestion() { free(domain_); }

  size_t domain_len_;
  char *domain_;
  unsigned short question_type_;
  unsigned short question_class_;
} DnsQuestion;

typedef struct Dns {
 public:
  Dns() = default;
  ~Dns() = default;

  DnsHeader header_;
  char question_[];
} Dns;

typedef struct DnsMessage {
 public:
  DnsMessage(size_t len, std::string domain)
      : domain_len_(len), domain_(domain) {}

  size_t domain_len_;
  std::string domain_;
} DnsMessage;

typedef struct DnsAnswer {
 public:
  DnsAnswer() = default;

  unsigned short name_;
  unsigned short answer_type_;
  unsigned short answer_class_;
  unsigned int time_to_live_;
  unsigned short data_len_;
  std::string ip_;
} DnsAnswer;

class AsyncDns {
 public:
  using ConnMap =
      std::unordered_map<std::string, std::shared_ptr<UdpConnection>>;

  AsyncDns();

  explicit AsyncDns(Looper<UdpConnection> *looper);

  AsyncDns(Looper<UdpConnection> *looper,
           std::shared_ptr<Ipv4Addr> dns_server_addr);

  void SetMessageCallBack(UdpConnection::MessageCallBack &&cb);
  void SetSendDataCallBack(UdpConnection::CallBack &&cb);
  void SetErrorCallBack(UdpConnection::CallBack &&cb);

  void StartLoop();
  void Loop();

  void AddDnsQuery(std::string &domain);
  void AddDnsQuery(const char *domain);

  void PrintQuery();

  DISALLOW_COPY_AND_ASSIGN(AsyncDns);

 private:
  void CreateDnsQuery(std::string domain, int len);

  DnsHeader *CreateHeader();

  DnsQuestion *CreateQuestion(std::string domain, int len);

  std::string ParseResponse(std::string &);

  bool quit_;
  int dns_socket_;
  ConnMap domain_to_conn_;
  Looper<UdpConnection> *looper_;
  Logger &log_;
  std::unordered_map<std::string, std::string> domain_to_ip_;
  std::thread dns_thread_;
  std::unique_ptr<SafeQueue<DnsMessage>> queue_domain_;
  std::shared_ptr<Ipv4Addr> dns_server_addr_;

  UdpConnection::CallBack new_conn_callback_;
  UdpConnection::MessageCallBack message_callback_;
  UdpConnection::CallBack send_data_callback_;
  UdpConnection::CallBack close_callback_;
  UdpConnection::CallBack error_callback_;
};

#endif  // CPPNET_NET_ASYNCDNS_H_
