//
// Created by v4kst1z
//

#ifndef CPPNET_BASE_SOCKET_H
#define CPPNET_BASE_SOCKET_H

extern "C" {
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#include "Common.h"
#include "Ipv4Addr.h"
#include "Logger.h"
#include "UdpConnection.h"

namespace sockets {
int CreateTcpSocket();

int CreateUdpSocket();

int CreateNonblockAndCloexecTcpSocket();

int CreateNonblockAndCloexecUdpSocket();

void SetNonBlock(int socket_fd);

void UnsetNonBlock(int socket_fd);

void Bind(int socket_fd, Ipv4Addr *addr);

void Listen(int socket_fd);

void Close(int socket_fd);

void SetReuseAddr(int socket_fd);

void SetReusePort(int socket_fd);

int Accept(int socket_fd, Ipv4Addr *addr);

int Connect(int sockfd, Ipv4Addr *addr);
void SendTo(int server_fd, Ipv4Addr *peer, const char *data, int len);

int RecvFrom(int server_fd, std::shared_ptr<UdpConnection> conn, int &error);
}  // namespace sockets

#endif  // CPPNET_BASE_SOCKET_H
