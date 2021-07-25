//
// Created by v4kst1z.
//

#include "../include/Client.h"

#include <cstring>
#include <memory>

#include "../include/Ipv4Addr.h"
#include "../include/Logger.h"
#include "../include/Server.h"
#include "../include/TimerManager.h"

void NewConnectionCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  std::cout << "client connect to " << peer->GetIp() << ":" << peer->GetPort()
            << std::endl;
}

void MessageCB(const std::shared_ptr<TcpConnection> &, IOBuffer &buf) {
  char buff[BUFSIZ];
  memcpy(buff, buf.GetReadAblePtr(), buf.GetReadAbleSize());
  buff[buf.GetReadAbleSize()] = '\x00';
  buf.ResetId();
  std::cout << "client receive message: " << buff << std::endl;
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

  Client client1 = Client(&l, addr, false);
  client1.SetNewConnCallback([](const std::shared_ptr<TcpConnection> &conn) {
    auto peer = conn->GetPeerAddr();
    std::cout << "client1 connect to " << peer->GetIp() << ":"
              << peer->GetPort() << std::endl;
  });
  client1.SetMessageCallBack(
      [](const std::shared_ptr<TcpConnection> &, IOBuffer &buf) {
        char buff[BUFSIZ];
        memcpy(buff, buf.GetReadAblePtr(), buf.GetReadAbleSize());
        buff[buf.GetReadAbleSize()] = '\x00';
        buf.ResetId();
        std::cout << "client1 receive message: " << buff << std::endl;
      });
  client1.Connect();
  client1.SendData("hello world\n");
  client1.SendData("hello world1\n");

  l.AddTimer(1000, [&]() { client.SendData("hello world2\n"); });
  l.AddTimer(7000, [&]() { client1.SendData("hello world4\n"); });
  l.AddTimer(2000, [&]() { client.SendData("hello world3\n"); });
  l.Loop();
  return 0;
}
