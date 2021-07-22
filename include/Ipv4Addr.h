//
// Created by v4kst1z
//

#ifndef CPPNET_BASE_IPV4ADDR_H
#define CPPNET_BASE_IPV4ADDR_H

extern "C" {
#include <netinet/in.h>
}

#include "Common.h"

class Ipv4Addr {
 public:
  Ipv4Addr() = default;

  explicit Ipv4Addr(unsigned short port);

  explicit Ipv4Addr(struct sockaddr_in *addr);

  Ipv4Addr(const char *addr, unsigned short port);

  void SetAddr(struct sockaddr_in *addr);

  struct sockaddr_in *GetAddr() {
    return &addr_;
  }

  unsigned short GetPort() const { return port_; }

  void SetPort(unsigned short port) { port_ = port; }

  std::string GetIp() const { return addr_str_; }

  void SetIp(const char *str) { addr_str_ = str; }

  DISALLOW_COPY_AND_ASSIGN(Ipv4Addr);

 private:
  bool IsIpv4(const std::string &addr);

  unsigned short port_;
  std::string addr_str_;
  struct sockaddr_in addr_ {};
};

#endif  // CPPNET_BASE_IPV4ADDR_H
