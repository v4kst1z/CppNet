//
// Created by v4kst1z.
//

#ifndef CPPNET_NET_CLIENT_H_
#define CPPNET_NET_CLIENT_H_

#include <Ipv4Addr.h>
#include <Looper.h>

class Client {
 public:
  Client(Looper *looper, std::shared_ptr<Ipv4Addr> addr);

  void Connect();

  void SetNewConnCallback(TcpConnection::CallBack &&cb);
  void SetMessageCallBack(TcpConnection::MessageCallBack &&cb);
  void SetCloseCallBack(TcpConnection::CallBack &&cb);
  void SetSendDataCallBack(TcpConnection::CallBack &&cb);
  void SetErrorCallBack(TcpConnection::CallBack &&cb);

  void SendData(const void *data, size_t len);
  void SendData(const std::string &message);
  void SendData(IOBuffer *buffer);

  void LoopStart();

  ~Client() = default;

  DISALLOW_COPY_AND_ASSIGN(Client);
 private:
  bool quit_;
  std::shared_ptr<Ipv4Addr> addr_;
  Looper *looper_;
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
