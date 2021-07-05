//
// Created by v4kst1z.
//

#include <Server.h>
#include <Ipv4Addr.h>
#include <Logger.h>

void NewConnectionCB(const std::shared_ptr<TcpConnection> &conn) {
  auto peer = conn->GetPeerAddr();
  DEBUG << "client at " << peer->GetIp() << ":" << peer->GetPort();
}

void MessageCB(const std::shared_ptr<TcpConnection> &conn, IOBuffer *buf) {
  char buff[BUFSIZ];
  memcpy(buff, buf->GetReadAblePtr(), buf->GetReadAbleSize());
  buff[buf->GetReadAbleSize()] = '\x00';
  conn->SendData(buff, buf->GetReadAbleSize());
  buf->ResetId();
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
  Server *server = new Server();
  server->SetNewConnCallback(NewConnectionCB);
  server->SetMessageCallBack(MessageCB);
  server->SetCloseCallBack(CloseCB);
  server->LoopStart();
  return 0;
}