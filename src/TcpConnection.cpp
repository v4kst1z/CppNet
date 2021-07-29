//
// Created by v4kst1z
//

#include "../include/TcpConnection.h"

#include <unistd.h>

#include <cstring>

#include "../include/IOBuffer.h"
#include "../include/Looper.h"
#include "../include/Socket.h"

TcpConnection::TcpConnection(int conn_fd, BaseLooper *looper,
                             std::shared_ptr<Ipv4Addr> addr)
    : conn_fd_(conn_fd),
      looper_(looper),
      perr_addr_(addr),
      conn_event_(Event(conn_fd)) {
  conn_event_.SetReadCallback(std::bind(&TcpConnection::OnRead, this));
  conn_event_.SetWriteCallback(std::bind(&TcpConnection::OnWrite, this));
  conn_event_.SetCloseCallback(std::bind(&TcpConnection::OnClose, this));
  conn_event_.EnableReadEvents(true);
}

void TcpConnection::SetNewConnCallback(const TcpConnection::CallBack &cb) {
  new_conn_callback_ = cb;
}

void TcpConnection::SetMessageCallBack(
    const TcpConnection::MessageCallBack &cb) {
  message_callback_ = cb;
}

void TcpConnection::SetCloseCallBack(const TcpConnection::CallBack &cb) {
  close_callback_ = cb;
}

void TcpConnection::SetSendDataCallBack(const TcpConnection::CallBack &cb) {
  send_data_callback_ = cb;
}

void TcpConnection::SetErrorCallBack(const TcpConnection::CallBack &cb) {
  error_callback_ = cb;
}

void TcpConnection::RunNewConnCallBack() {
  if (new_conn_callback_) new_conn_callback_(shared_from_this());
}

void TcpConnection::RunMessageCallBack() {
  if (message_callback_) message_callback_(shared_from_this(), input_buffer_);
}

void TcpConnection::RunCloseCallBack() {
  if (close_callback_) close_callback_(shared_from_this());
}

void TcpConnection::RunSendDataCallBack() {
  if (send_data_callback_) send_data_callback_(shared_from_this());
}

void TcpConnection::RunErrorCallBack() {
  if (error_callback_) error_callback_(shared_from_this());
}

const std::shared_ptr<Ipv4Addr> TcpConnection::GetPeerAddr() const {
  return perr_addr_;
}

int TcpConnection::GetConnFd() { return conn_fd_; }

void TcpConnection::OnRead() {
  char buf[BUFSIZ];
  int sum_len = 0;
  while (true) {
    memset(buf, '\x00', BUFSIZ);
    int len = read(GetConnFd(), buf, BUFSIZ);
    if (len > 0) {
      input_buffer_.AppendData(buf, len);
      sum_len += len;
    } else if (len < 0) {
      input_buffer_.AppendData(buf, strlen(buf));
      sum_len += strlen(buf);
      if (errno == EAGAIN)  // 缓冲区为空
        break;
      else if (errno == EINTR)  //中断信号
        continue;
      else if (errno == ECONNRESET) {  // 客户端已经close，如果继续读会
                                       // Connection reset by peer
        RunErrorCallBack();
        break;
      } else {
        ERROR << "read error callback " << errno;
        break;
      }
    } else {
      OnClose();
      return;
    }
  }

  RunMessageCallBack();
}

void TcpConnection::OnWrite() {
  SendData(output_buffer.GetReadAblePtr(), output_buffer.GetReadAbleSize());
}

void TcpConnection::OnClose() {
  if (output_buffer.GetReadAbleSize()) {
    while (output_buffer.GetReadAbleSize()) SendData(&output_buffer);
  }
  RunCloseCallBack();

  std::shared_ptr<VariantEventBase> fd = looper_->GetEventPtr(conn_fd_);
  looper_->DelEvent(fd);
  looper_->EraseConn(conn_fd_);
}

void TcpConnection::SendData(const void *data, size_t len, bool del) {
  // 1. data 是栈上临时数据 del = false
  // 2. data 是堆上数据 del = true
  // 3. data 是 io buffer 中的数据 del = false
  if (looper_->GetThreadId() != std::this_thread::get_id()) {
    void *buff;
    if (!del) {
      buff = malloc(len);
      memcpy(buff, data, len);
    } else {
      buff = const_cast<void *>(data);
    }
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        shared_from_this(), buff, len, true));
    return;
  }

  size_t send_data_len = 0;

  if (output_buffer.GetReadAblePtr() != data &&
      output_buffer.GetReadAbleSize()) {
    //非 io buffer 数据 压入 io buffer
    output_buffer.AppendData(static_cast<const char *>(data), len);
    if (del) free(const_cast<void *>(data));
    EnableWrite(true);
    return;
  }

  while (true) {
    int write_len = write(conn_fd_, (const char *)data + send_data_len,
                          len - send_data_len);
    if (write_len > 0) {
      send_data_len += write_len;
      if (len == send_data_len) {
        //发送完成
        EnableWrite(false);
        RunSendDataCallBack();
        output_buffer.ResetId();
        break;
      };
    } else if (write_len < 0) {
      if (errno == EAGAIN) {  // 没有数据可读
        output_buffer.AppendData((const char *)data + send_data_len,
                                 len - send_data_len);
        EnableWrite(true);
        break;
      } else if (errno == EINTR) {  // 操作被中断
        continue;
      } else if (errno == EPIPE) {
        //客户端已经close，如果继续写会 EPIPE
        RunErrorCallBack();
        break;
      } else {
        ERROR << "write error callback " << errno;
        break;
      }
    }
  }
  if (del) free(const_cast<void *>(data));
}

void TcpConnection::SendData(const std::string &message) {
  if (looper_->GetThreadId() != std::this_thread::get_id()) {
    int len = message.size();
    char *buff = (char *)malloc(len);
    memcpy(buff, message.data(), len);
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        shared_from_this(), buff, len, true));
    return;
  }
  SendData(output_buffer.GetReadAblePtr(), output_buffer.GetReadAbleSize());
  if (output_buffer.GetReadAbleSize()) {
    output_buffer.AppendData(static_cast<const char *>(message.data()),
                             message.size());
    return;
  }
  SendData(message.data(), message.size());
}

void TcpConnection::SendData(IOBuffer *buffer) {
  int len = buffer->GetReadAbleSize();
  if (looper_->GetThreadId() != std::this_thread::get_id()) {
    char *buff = (char *)malloc(len);
    memcpy(buff, buffer->GetReadAblePtr(), len);
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        shared_from_this(), buff, len, true));
    buffer->ResetId();
    return;
  }
  if (output_buffer.GetReadAbleSize()) {
    output_buffer.AppendData(buffer->GetReadAblePtr(), len);
    buffer->ResetId();
    return;
  }
  SendData(output_buffer.GetReadAblePtr(), output_buffer.GetReadAbleSize());
}

void TcpConnection::EnableWrite(bool flag) {
  auto event = looper_->GetEventPtr(conn_fd_);
  if (!event) return;
  event->Visit([flag](EventBase<Event> &conn_event) {
    conn_event.EnableWriteEvents(flag);
  });

  looper_->ModEvent(event);
}

EventBase<Event> &TcpConnection::GetEvent() { return conn_event_; }

TcpConnection::~TcpConnection() { sockets::Close(conn_fd_); }

ThreadPool *TcpConnection::GetThreadPoolPtr() { return looper_->GetTPollPtr(); }

void TcpConnection::SetEvent(int event) {
  conn_event_.EnableReadEvents(false);
  conn_event_.SetEvent(event);
}
