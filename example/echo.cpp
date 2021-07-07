//
// Created by v4kst1z.
//

#include <Server.h>
#include <Ipv4Addr.h>
#include <Logger.h>

void NewConnectionCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "client at " << peer->GetIp() << ":" << peer->GetPort();
  conn->SendData("hello~\n");
}

void MessageCB(const std::shared_ptr<TcpConnection> &conn, IOBuffer &buf) {
  conn->SendData(buf.GetReadAblePtr(), buf.GetReadAbleSize());
  buf.ResetId();
}

void CloseCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "close at " << peer->GetIp() << ":" << peer->GetPort();
}

void ErrorCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "error at " << peer->GetIp() << ":" << peer->GetPort();
}

int main() {
  Server server = Server();
  server.SetNewConnCallback(NewConnectionCB);
  server.SetMessageCallBack(MessageCB);
  server.SetCloseCallBack(CloseCB);
  server.LoopStart();
  return 0;
}