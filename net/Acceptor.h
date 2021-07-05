//
// Created by v4kst1z
//

#ifndef CPPNET_ACCEPTOR_H
#define CPPNET_ACCEPTOR_H

#include <memory>

#include <Event.h>
#include <Socket.h>
#include <Ipv4Addr.h>
#include <TcpConnection.h>
#include <Looper.h>

class Acceptor {
 public:
  using NewConnectionCallBack = std::function<void(std::shared_ptr<TcpConnection>)>;

  Acceptor(Ipv4Addr *addr, Looper *looper) :
      listen_addr_(addr),
      looper_(looper),
      accept_fd_(sockets::CreateNonblockAndCloexecTcpSocket()),
      accept_event_(EventBase<Event>(accept_fd_)) {
    sockets::SetReuseAddr(accept_fd_);
    sockets::SetReusePort(accept_fd_);
    sockets::Bind(accept_fd_, listen_addr_);
    accept_event_.SetReadCallback(std::bind(&Acceptor::HandelNewConnection, this));
  }

  void SetNewConnectionCallBack(NewConnectionCallBack &&cb) {
    new_conn_cb_ = cb;
  }

  void Listen() {
    sockets::Listen(accept_fd_);
    accept_event_.EnableReadEvents(true);
  }

  int GetFd() {
    return accept_fd_;
  }

  EventBase<Event> GetEvent() {
    return accept_event_;
  }

  Looper *GetLooper() {
    return looper_;
  }
 private:
  void HandelNewConnection() {
    while (true) {
      auto peer_addr = std::make_shared<Ipv4Addr>(0);
      int conn_fd = sockets::Accept(accept_fd_, peer_addr.get());
      if (!conn_fd) break;
      auto conn = std::make_shared<TcpConnection>(conn_fd, looper_, peer_addr);
      looper_->InsertConn(conn_fd, conn);
      if (conn_fd) {
        if (new_conn_cb_)
          new_conn_cb_(conn);
        else
          sockets::Close(conn_fd);
      }
    }
  }

  Looper *looper_;
  int accept_fd_;
  EventBase<Event> accept_event_;
  NewConnectionCallBack new_conn_cb_;
  Ipv4Addr *listen_addr_;
};

#endif //CPPNET_ACCEPTOR_H
