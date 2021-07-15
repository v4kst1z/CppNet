//
// Created by v4kst1z
//

extern "C" {
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <cstring>

#include "../include/Common.h"
#include "../include/Ipv4Addr.h"
#include "../include/Logger.h"
#include "../include/UdpConnection.h"

namespace sockets {
int CreateTcpSocket() {
  int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_fd < 0) ERROR << "create socket failed!";
  return socket_fd;
}

int CreateUdpSocket() {
  int socket_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_fd < 0) ERROR << "create socket failed!";
  return socket_fd;
}

int CreateNonblockAndCloexecTcpSocket() {
  int socket_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                           IPPROTO_TCP);
  if (socket_fd < 0) ERROR << "create socket failed!";
  return socket_fd;
}

int CreateNonblockAndCloexecUdpSocket() {
  int socket_fd =
      ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
  ;
  if (socket_fd < 0) ERROR << "create socket failed!";
  return socket_fd;
}

void SetNonBlock(int socket_fd) {
  int opts = fcntl(socket_fd, F_GETFL);
  if (opts < 0) ERROR << "fcntl get failed!";

  if (fcntl(socket_fd, F_SETFL, opts | O_NONBLOCK) < 0)
    ERROR << "fcntl set failed!";
}

void UnsetNonBlock(int socket_fd) {
  int opts = fcntl(socket_fd, F_GETFL);
  if (opts < 0) ERROR << "fcntl get failed!";

  if (fcntl(socket_fd, F_SETFL, opts | ~O_NONBLOCK) < 0)
    ERROR << "fcntl set failed!";
}

void Bind(int socket_fd, Ipv4Addr *addr) {
  struct sockaddr_in *bind_addr = addr->GetAddr();
  if (bind(socket_fd, (struct sockaddr *)bind_addr, sizeof(*bind_addr)) < 0)
    ERROR << "bind failed!";
}

void Listen(int socket_fd) {
  if (listen(socket_fd, SOMAXCONN) < 0) ERROR << "listen failed!";
}

void Close(int socket_fd) {
  if (close(socket_fd) < 0) ERROR << "close failed!";
}

void SetReuseAddr(int socket_fd) {
  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0)
    ERROR << "setsockopt failed!";
}

void SetReusePort(int socket_fd) {
  int optval = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) <
      0)
    ERROR << "setsockopt failed!";
}

int Accept(int socket_fd, Ipv4Addr *addr) {
  struct sockaddr_in *bind_addr = addr->GetAddr();
  char ip_str[INET_ADDRSTRLEN];
  socklen_t addrlen = sizeof(*bind_addr);
  int conn_fd = accept4(socket_fd, (struct sockaddr *)bind_addr, &addrlen,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
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

int Connect(int sockfd, Ipv4Addr *addr) {
  struct sockaddr_in *server_addr = addr->GetAddr();
  socklen_t addrlen = sizeof(*server_addr);
  int ret = connect(sockfd, (struct sockaddr *)server_addr, addrlen);
  if (ret < 0 && errno != EINPROGRESS) ERROR << "connect failed!" << errno;
  return ret;
}

void SendTo(int server_fd, Ipv4Addr *peer, const char *data, int len) {
  struct sockaddr_in *dst = peer->GetAddr();
  ssize_t ret = sendto(server_fd, data, len, 0, (const struct sockaddr *)dst,
                       sizeof(*dst));
  if (ret < 0) {
    ERROR << "SendTo failed "
          << "error num is " << errno;
  }
}

int RecvFrom(int server_fd, std::shared_ptr<UdpConnection> conn, int &error) {
  struct sockaddr_in clinet_addr;
  socklen_t len = sizeof(clinet_addr);
  char buf[BUFSIZ];
  int ret = recvfrom(server_fd, buf, BUFSIZ, 0, (struct sockaddr *)&clinet_addr,
                     &len);
  if (ret > 0) {
    conn->Read(buf, strlen(buf));
    conn->SetPeerAddr(&clinet_addr);
  } else if (ret < 0) {
    error = errno;
  }
  return ret;
}
}  // namespace sockets