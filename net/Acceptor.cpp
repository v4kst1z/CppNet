//
// Created by v4kst1z
//

#include "Acceptor.h"

#include "Looper.h"
#include "TcpConnection.h"

Acceptor::Acceptor(Ipv4Addr *addr, Looper<TcpConnection> *looper)
    : listen_addr_(addr),
      looper_(looper),
      accept_fd_(sockets::CreateNonblockAndCloexecTcpSocket()),
      accept_event_(EventBase<Event>(accept_fd_)) {
  sockets::SetReuseAddr(accept_fd_);
  sockets::SetReusePort(accept_fd_);
  sockets::Bind(accept_fd_, listen_addr_);
  looper_->SetServerFd(accept_fd_);
  accept_event_.SetReadCallback(
      std::bind(&Acceptor::HandelNewConnection, this));
}

void Acceptor::HandelNewConnection() {
  while (true) {
    auto peer_addr = std::make_shared<Ipv4Addr>(0);
    int conn_fd = sockets::Accept(accept_fd_, peer_addr.get());
    if (!conn_fd) break;
    auto conn = std::make_shared<TcpConnection>(
        conn_fd, dynamic_cast<BaseLooper *>(looper_), peer_addr);
    looper_->InsertConn(conn_fd, conn);
    if (conn_fd) {
      if (new_conn_cb_)
        new_conn_cb_(conn);
      else
        sockets::Close(conn_fd);
    }
  }
}

void Acceptor::SetNewConnectionCallBack(NewConnectionCallBack &&cb) {
  new_conn_cb_ = cb;
}

void Acceptor::Listen() {
  sockets::Listen(accept_fd_);
  accept_event_.EnableReadEvents(true);
}

int Acceptor::GetFd() { return accept_fd_; }

EventBase<Event> Acceptor::GetEvent() { return accept_event_; }

Looper<TcpConnection> *Acceptor::GetLooper() { return looper_; }