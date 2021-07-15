
//
// Created by v4kst1z
//

#include <cstring>

#include "../include/Ipv4Addr.h"
#include "../include/Logger.h"
#include "../include/UdpServer.h"

void MessageCB(const std::shared_ptr<UdpConnection> &conn, IOBuffer &buf) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "receive data from " << peer->GetIp() << ":" << peer->GetPort();

  char buff[BUFSIZ];
  memcpy(buff, buf.GetReadAblePtr(), buf.GetReadAbleSize());
  buff[buf.GetReadAbleSize()] = '\x00';
  DEBUG << "receive test " << buff << std::endl;
  conn->SendData(buff, buf.GetReadAbleSize());
  buf.ResetId();
}

void SendDataCB(const std::shared_ptr<UdpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "send data to " << peer->GetIp() << ":" << peer->GetPort()
        << " complete";
}

int main() {
  UdpServer server = UdpServer();
  server.SetMessageCallBack(MessageCB);
  server.SetSendDataCallBack(SendDataCB);
  server.LoopStart();
  return 0;
}