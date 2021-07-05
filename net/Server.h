//
// Created by v4kst1z
//

#ifndef CPPNET_SERVER_H
#define CPPNET_SERVER_H

#include <Looper.h>
#include <SafeQueue.h>

class Logger;

class Server {
 public:
  explicit Server(int io_threads_num = 8, int timer_num = 3, unsigned short port = 8888);

  void SetNewConnCallback(TcpConnection::CallBack &&cb);
  void SetMessageCallBack(TcpConnection::MessageCallBack &&cb);
  void SetCloseCallBack(TcpConnection::CallBack &&cb);
  void SetSendDataCallBack(TcpConnection::CallBack &&cb);
  void SetErrorCallBack(TcpConnection::CallBack &&cb);

  void AddTimer(int timeout, std::function<void()> fun);

  void LoopStart();

  DISALLOW_COPY_AND_ASSIGN(Server);
 private:
  unsigned short server_port_;
  Ipv4Addr *server_addr_;
  std::shared_ptr<TimerManager> timer_manager_;
  std::vector<Looper *> io_threads_;
  Looper *main_thread_;

  TcpConnection::CallBack new_conn_callback_;
  TcpConnection::MessageCallBack message_callback_;
  TcpConnection::CallBack send_data_callback_;
  TcpConnection::CallBack close_callback_;
  TcpConnection::CallBack error_callback_;
  Logger &log_;
};

#endif //CPPNET_SERVER_H
