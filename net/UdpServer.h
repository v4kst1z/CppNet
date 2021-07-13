//
// Created by v4kst1z.
//

#ifndef CPPNET_UDPSERVER_H
#define CPPNET_UDPSERVER_H

#include <functional>
#include <memory>

#include "Event.h"
#include "IOBuffer.h"

class UdpConnection;
class ThreadPool;
class Logger;
class Ipv4Addr;
class TimerManager;

template <typename T>
class Looper;

typedef struct PeerData {
  std::unique_ptr<Ipv4Addr> addr_;
  IOBuffer output_buffer_;
  IOBuffer input_buffer_;
} PeerData;

class UdpServer {
 public:
  using MessageCallBack = std::function<void(const std::shared_ptr<PeerData>)>;
  using CallBack = std::function<void(const std::shared_ptr<PeerData>)>;

  explicit UdpServer(int io_threads_num = 8, int timer_num = 3,
                     unsigned short port = 8888, uint8_t tpool_num = 0);

  void SetMessageCallBack(const MessageCallBack &&cb);
  void SetSendDataCallBack(const CallBack &&cb);

  void RunMessageCallBack();
  void RunSendDataCallBack();

  void OnRead();
  void OnWrite();

  void AddTimer(int timeout, std::function<void()> fun);

  void SendData(const void *data, size_t len);
  void SendData(const std::string &message);
  void SendData(IOBuffer *buffer);

  const std::shared_ptr<Ipv4Addr> GetPeerAddr() const;

  ~UdpServer();

 private:
  std::shared_ptr<Ipv4Addr> server_addr_;
  std::vector<Looper<UdpConnection> *> io_threads_;
  Looper<UdpConnection> *main_thread_;
  std::shared_ptr<TimerManager> timer_manager_;

  std::unique_ptr<ThreadPool> tpool_;
  MessageCallBack message_callback_;
  CallBack send_data_callback_;
  CallBack error_callback_;

  Logger &log_;
};

#endif  // CPPNET_UDPSERVER_H
