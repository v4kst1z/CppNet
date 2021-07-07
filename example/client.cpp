//
// Created by v4kst1z.
//

#include <Server.h>
#include <Ipv4Addr.h>
#include <Logger.h>
#include <Client.h>

void NewConnectionCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  std::cout << "connect to " << peer->GetIp() << ":" << peer->GetPort() << std::endl;
  conn->SendData("client test");
}

void MessageCB(const std::shared_ptr<TcpConnection> &conn, IOBuffer &buf) {
  char buff[BUFSIZ];
  memcpy(buff, buf.GetReadAblePtr(), buf.GetReadAbleSize());
  buff[buf.GetReadAbleSize()] = '\x00';
  buf.ResetId();
  std::cout << "receive message: " << buff << std::endl;
}

int main() {
  Ipv4Addr addr = Ipv4Addr("127.0.0.1", 8888);
  Client client = Client(&addr);
  client.SetNewConnCallback(NewConnectionCB);
  client.SetMessageCallBack(MessageCB);
  client.LoopStart();
  return 0;
}