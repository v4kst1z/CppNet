//
// Created by v4kst1z
//

#ifndef CPPNET_NET_UDPCONNECTION_H_
#define CPPNET_NET_UDPCONNECTION_H_

#include <functional>
#include <memory>

#include "Event.h"
#include "IOBuffer.h"

class Ipv4Addr;
class BaseLooper;

class UdpConnection : public std::enable_shared_from_this<UdpConnection> {
 public:
  using MessageCallBack =
      std::function<void(const std::shared_ptr<UdpConnection> &, IOBuffer &)>;
  using CallBack = std::function<void(const std::shared_ptr<UdpConnection> &)>;

  UdpConnection(BaseLooper *);

  void SetMessageCallBack(const MessageCallBack &cb);
  void SetSendDataCallBack(const CallBack &cb);
  void SetErrorCallBack(const CallBack &cb);

  void RunMessageCallBack();
  void RunSendDataCallBack();
  void RunErrorCallBack();

  void Read(const char *data, size_t len);
  void Write(const char *data, size_t len);

  void SendData(const void *data, size_t len);
  void SendData(const std::string &message);
  void SendData(IOBuffer *buffer);

  const std::shared_ptr<Ipv4Addr> GetPeerAddr() const;
  void SetPeerAddr(struct sockaddr_in *);

  ~UdpConnection() = default;

 private:
  BaseLooper *looper_;
  std::shared_ptr<Ipv4Addr> perr_addr_;

  MessageCallBack message_callback_;
  CallBack send_data_callback_;
  CallBack error_callback_;

  IOBuffer input_buffer_;
};

#endif  // CPPNET_NET_UDPCONNECTION_H_
