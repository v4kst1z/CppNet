//
// Created by v4kst1z
//

#ifndef CPPNET_SOCKET_H
#define CPPNET_SOCKET_H

extern "C" {
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
}

#include <Common.h>
#include <Ipv4Addr.h>
#include <Logger.h>

namespace sockets {
inline int CreateTcpSocket() {
  int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd < 0)
    ERROR << "create socket failed!";
  return socket_fd;
}

inline int CreateNonblockAndCloexecTcpSocket() {
  int socket_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (socket_fd < 0)
	ERROR << "create socket failed!";
  return socket_fd;
}

inline void SetNonBlock(int socket_fd) {
  int opts = fcntl(socket_fd, F_GETFL);
  if (opts < 0)
	ERROR << "fcntl get failed!";

  if (fcntl(socket_fd, F_SETFL, opts | O_NONBLOCK) < 0)
	ERROR << "fcntl set failed!";
}

inline void UnsetNonBlock(int socket_fd) {
  int opts = fcntl(socket_fd, F_GETFL);
  if (opts < 0)
	ERROR << "fcntl get failed!";

  if (fcntl(socket_fd, F_SETFL, opts | ~O_NONBLOCK) < 0)
	ERROR << "fcntl set failed!";
}

inline void Bind(int socket_fd, Ipv4Addr *addr) {
  struct sockaddr_in *bind_addr = addr->GetAddr();
  if (bind(socket_fd, (struct sockaddr *)bind_addr, sizeof(*bind_addr)) < 0)
	ERROR << "bind failed!";
}

inline void Listen(int socket_fd) {
  if (listen(socket_fd, SOMAXCONN) < 0)
    ERROR << "listen failed!";
}

inline void Close(int socket_fd) {
  if (close(socket_fd) < 0)
    ERROR << "close failed!";
}

inline void SetReuseAddr(int socket_fd) {
  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	ERROR << "setsockopt failed!";
}

inline void SetReusePort(int socket_fd) {
  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
	ERROR << "setsockopt failed!";
}

inline int Accept(int socket_fd, Ipv4Addr *addr) {
  struct sockaddr_in *bind_addr = addr->GetAddr();
  char ip_str[INET_ADDRSTRLEN];
  socklen_t addrlen = sizeof(*bind_addr);
  int conn_fd = accept4(socket_fd, (struct sockaddr *)bind_addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (conn_fd < 0) {
	if (errno == EAGAIN)
	  conn_fd = 0;
	else
	  ERROR << "accept4 failed!";
  }
  addr->SetPort(ntohs(bind_addr->sin_port));
  inet_ntop(AF_INET, &bind_addr->sin_addr.s_addr, ip_str, INET_ADDRSTRLEN);
  addr->SetIp(ip_str);
  return conn_fd;
}

}

#endif //CPPNET_SOCKET_H
