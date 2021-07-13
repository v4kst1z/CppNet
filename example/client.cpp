//
// Created by v4kst1z.
//

#include <memory>

#include "TimerManager.h"
#include "Server.h"
#include "Ipv4Addr.h"
#include "Logger.h"
#include "Client.h"

void NewConnectionCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  std::cout << "connect to " << peer->GetIp() << ":" << peer->GetPort() << std::endl;
}

void MessageCB(const std::shared_ptr<TcpConnection> &conn, IOBuffer &buf) {
  char buff[BUFSIZ];
  memcpy(buff, buf.GetReadAblePtr(), buf.GetReadAbleSize());
  buff[buf.GetReadAbleSize()] = '\x00';
  buf.ResetId();
  std::cout << "receive message: " << buff << std::endl;
}

int main() {
  auto tm = std::make_shared<TimerManager>();
  Looper<TcpConnection> l = Looper<TcpConnection>(tm);
  auto addr = std::make_shared<Ipv4Addr>("127.0.0.1", 8888);
  Client client = Client(&l, addr);
  client.SetNewConnCallback(NewConnectionCB);
  client.SetMessageCallBack(MessageCB);
  client.Connect();
  client.SendData("hello world\n");
  client.SendData("hello world1\n");
  l.AddTimer(1000, [&]() {
    client.SendData("hello world2\n");
  });
  l.AddTimer(7000, [&]() {
    client.SendData("hello world4\n");
  });
  l.AddTimer(2000, [&]() {
    client.SendData("hello world3\n");
  });
  l.Loop();
  return 0;
}