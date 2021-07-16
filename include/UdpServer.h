//
// Created by v4kst1z
//

#ifndef CPPNET_NET_UDPSERVER_H
#define CPPNET_NET_UDPSERVER_H

#include <functional>
#include <memory>

#include "Common.h"
#include "Event.h"
#include "IOBuffer.h"
#include "UdpConnection.h"

class UdpConnection;
class ThreadPool;
class Logger;
class Ipv4Addr;
class TimerManager;

template <typename T>
class Looper;

class UdpServer {
 public:
  explicit UdpServer(int io_threads_num = 8, int timer_num = 3,
                     unsigned short port = 7777, uint8_t tpool_num = 0);

  void SetMessageCallBack(const UdpConnection::MessageCallBack &&cb);
  void SetSendDataCallBack(const UdpConnection::CallBack &&cb);
  void SetErrorCallBack(const UdpConnection::CallBack &&cb);

  void AddTimer(int timeout, std::function<void()> fun);

  ThreadPool *GetThreadPoolPtr();

  Looper<UdpConnection> *GetMianLoop();
  std::vector<Looper<UdpConnection> *> GetIoLoop();

  unsigned short GetPort();
  Ipv4Addr *GetAddr();

  void LoopStart();

  void Exit();

  ~UdpServer();
  DISALLOW_COPY_AND_ASSIGN(UdpServer);

 private:
  bool quit_;
  int server_port_;
  Ipv4Addr *server_addr_;
  std::shared_ptr<TimerManager> timer_manager_;
  std::vector<Looper<UdpConnection> *> io_threads_;
  Looper<UdpConnection> *main_thread_;

  std::unique_ptr<ThreadPool> tpool_;
  UdpConnection::MessageCallBack message_callback_;
  UdpConnection::CallBack send_data_callback_;
  UdpConnection::CallBack error_callback_;

  Logger &log_;
};

#endif  // CPPNET_NET_UDPSERVER_H
