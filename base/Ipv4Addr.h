//
// Created by v4kst1z
//

#ifndef CPPNET_IPV4ADDR_H
#define CPPNET_IPV4ADDR_H

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
}

#include <string>
#include <regex>

#include <Common.h>
#include <Logger.h>

class Ipv4Addr {
 public:
  explicit Ipv4Addr(unsigned short port) :
      port_(port),
      addr_str_("127.0.0.1") {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
  }

  explicit Ipv4Addr(struct sockaddr_in *addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);

    port_ = ntohs(addr_.sin_port);
    addr_str_ = ip_str;
    addr_ = *addr;
  }

  Ipv4Addr(const char *addr, unsigned short port) :
      port_(port),
      addr_str_(addr) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    if (IsIpv4(addr_str_))
      addr_.sin_addr.s_addr = inet_addr(addr_str_.c_str());
    else {
      struct hostent *he;
      char ip_str[INET_ADDRSTRLEN];

      if ((he = gethostbyname(addr_str_.c_str())) == NULL)
        ERROR << "gethostbyname failed";

      memcpy(&addr_.sin_addr, he->h_addr_list[0], he->h_length);
      inet_ntop(AF_INET, he->h_addr_list[0], ip_str, INET_ADDRSTRLEN);
      addr_str_ = ip_str;
    }

  }

  struct sockaddr_in *GetAddr() {
    return &addr_;
  }

  const unsigned short GetPort() const {
    return port_;
  }

  void SetPort(unsigned short port) {
    port_ = port;
  }

  const std::string GetIp() const {
    return addr_str_;
  }

  void SetIp(const char *str) {
    addr_str_ = str;
  }

  DISALLOW_COPY_AND_ASSIGN(Ipv4Addr);
 private:
  bool IsIpv4(const std::string &addr) {
    std::regex pattern(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    if (std::regex_match(addr, pattern))
      return true;
    else
      return false;
  }

  unsigned short port_;
  std::string addr_str_;
  struct sockaddr_in addr_{};
};

#endif //CPPNET_IPV4ADDR_H
