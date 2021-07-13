//
// Created by v4kst1z
//

#include "UdpConnection.h"

#include "Logger.h"
#include "Looper.h"

UdpConnection::UdpConnection(BaseLooper *looper)
    : looper_(looper), perr_addr_(new Ipv4Addr()) {}

void UdpConnection::SetMessageCallBack(const MessageCallBack &cb) {
  message_callback_ = cb;
}

void UdpConnection::SetSendDataCallBack(const CallBack &cb) {
  send_data_callback_ = cb;
}

void UdpConnection::SetErrorCallBack(const CallBack &cb) {
  error_callback_ = cb;
}

void UdpConnection::RunMessageCallBack() {
  if (message_callback_) message_callback_(shared_from_this(), input_buffer_);
}

void UdpConnection::RunSendDataCallBack() {
  if (send_data_callback_) send_data_callback_(shared_from_this());
}

void UdpConnection::RunErrorCallBack() {
  if (error_callback_) error_callback_(shared_from_this());
}

void UdpConnection::Read(const char *data, size_t len) {
  input_buffer_.AppendData(data, len);
}

void UdpConnection::Write(const char *data, size_t len) {
  sockets::SendTo(looper_->GetServerFd(), perr_addr_.get(), data, len);
  RunSendDataCallBack();
}

void UdpConnection::SendData(const void *data, size_t len) {
  Write(static_cast<const char *>(data), len);
}

void UdpConnection::SendData(const std::string &message) {
  Write(message.data(), message.size());
}

void UdpConnection::SendData(IOBuffer *buffer) {
  Write(buffer->GetWriteAblePtr(), buffer->GetWriteAbleSize());
}

const std::shared_ptr<Ipv4Addr> UdpConnection::GetPeerAddr() const {
  return perr_addr_;
}

void UdpConnection::SetPeerAddr(struct sockaddr_in *addr) {
  perr_addr_->SetAddr(addr);
}
