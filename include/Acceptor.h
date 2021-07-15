//
// Created by v4kst1z
//

#ifndef CPPNET_NET_ACCEPTOR_H
#define CPPNET_NET_ACCEPTOR_H

#include <memory>

#include "Event.h"
#include "Ipv4Addr.h"
#include "Socket.h"

class TcpConnection;

template <typename T>
class Looper;

class Acceptor {
 public:
  using NewConnectionCallBack =
      std::function<void(std::shared_ptr<TcpConnection>)>;

  Acceptor(Ipv4Addr *addr, Looper<TcpConnection> *looper);

  void SetNewConnectionCallBack(NewConnectionCallBack &&cb);

  void Listen();

  int GetFd();

  EventBase<Event> GetEvent();

  Looper<TcpConnection> *GetLooper();

 private:
  void HandelNewConnection();

  Looper<TcpConnection> *looper_;
  int accept_fd_;
  EventBase<Event> accept_event_;
  NewConnectionCallBack new_conn_cb_;
  Ipv4Addr *listen_addr_;
};

#endif  // CPPNET_NET_ACCEPTOR_H
