//
// Created by v4kst1z.
//

#ifndef CPPNET_NET_CLIENT_H_
#define CPPNET_NET_CLIENT_H_

#include <Ipv4Addr.h>
#include <Looper.h>

class Client {
 public:
  explicit Client(Ipv4Addr *addr);

  void SetNewConnCallback(TcpConnection::CallBack &&cb);
  void SetMessageCallBack(TcpConnection::MessageCallBack &&cb);
  void SetCloseCallBack(TcpConnection::CallBack &&cb);
  void SetSendDataCallBack(TcpConnection::CallBack &&cb);
  void SetErrorCallBack(TcpConnection::CallBack &&cb);

  void LoopStart();

  ~Client() = default;

  DISALLOW_COPY_AND_ASSIGN(Client);
 private:
  bool quit_;
  std::shared_ptr<Ipv4Addr> addr_;
  std::shared_ptr<Looper> looper_;
  Logger &log_;
  int conn_fd_;
  std::shared_ptr<TcpConnection> conn_;

  TcpConnection::CallBack new_conn_callback_;
  TcpConnection::MessageCallBack message_callback_;
  TcpConnection::CallBack send_data_callback_;
  TcpConnection::CallBack close_callback_;
  TcpConnection::CallBack error_callback_;
};

#endif //CPPNET_NET_CLIENT_H_
