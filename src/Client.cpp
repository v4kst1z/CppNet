//
// Created by v4kst1z
//

#include "../include/Client.h"

#include <cstring>
#include <memory>

#include "../include/Looper.h"
#include "../include/Socket.h"
#include "../include/TcpConnection.h"

Client::Client(Looper<TcpConnection> *looper, std::shared_ptr<Ipv4Addr> addr,
               bool log, bool async)
    : connect_(false),
      addr_(addr),
      looper_(looper),
      log_(Logger::GetInstance()),
      conn_fd_(sockets::CreateNonblockAndCloexecTcpSocket()),
      conn_(std::make_shared<TcpConnection>(
          conn_fd_, dynamic_cast<BaseLooper *>(looper_), addr_)) {
  looper_->InsertConn(conn_fd_, conn_);
  if (!async) looper_->SetLoopId(std::this_thread::get_id());
  if (log) log_.Start();
}

void Client::SetNewConnCallback(TcpConnection::CallBack &&cb) {
  new_conn_callback_ = std::move(cb);
}

void Client::SetMessageCallBack(TcpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
}

void Client::SetCloseCallBack(TcpConnection::CallBack &&cb) {
  close_callback_ = std::move(cb);
}

void Client::SetSendDataCallBack(TcpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
}

void Client::SetErrorCallBack(TcpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
}

void Client::LoopStart() { looper_->Loop(); }

void Client::Connect() {
  int error = 0;
  int ret = sockets::Connect(conn_fd_, addr_.get(), error);
  DEBUG << "connect to " << addr_->GetIp() << ":" << addr_->GetPort();
  if (ret < 0) {
    if (error == EINPROGRESS) {
      conn_->SetMessageCallBack(
          [this](const std::shared_ptr<TcpConnection> &conn, IOBuffer &) {
            int result;
            socklen_t result_len = sizeof(result);
            if (getsockopt(conn->GetConnFd(), SOL_SOCKET, SO_ERROR, &result,
                           &result_len) < 0) {
              DEBUG << "getsockopt error ~";
              return;
            }

            if (result != 0) {
              DEBUG << "Connect error ~ " << result;
              return;
            }

            conn->SetNewConnCallback(new_conn_callback_);
            conn->SetCloseCallBack(close_callback_);
            conn->SetErrorCallBack(error_callback_);
            conn->SetSendDataCallBack(send_data_callback_);
            conn->SetMessageCallBack(message_callback_);

            connect_ = true;
            looper_->AddTasks(tasks_);
            conn->RunNewConnCallBack();
            conn->EnableWrite(false);
          });

      conn_->GetEvent().EnableWriteEvents(true);
      looper_->AddEvent(std::make_shared<VariantEventBase>(conn_->GetEvent()));
    }
  } else if (ret == 0) {
    conn_->SetNewConnCallback(new_conn_callback_);
    conn_->SetCloseCallBack(close_callback_);
    conn_->SetErrorCallBack(error_callback_);
    conn_->SetSendDataCallBack(send_data_callback_);
    conn_->SetMessageCallBack(message_callback_);

    connect_ = true;
    conn_->RunNewConnCallBack();
  }
}

void Client::SendData(const void *data, size_t len, bool del) {
  void *buff;
  if (!del) {
    buff = malloc(len);
    memcpy(buff, const_cast<void *>(data), len);
  } else {
    buff = const_cast<void *>(data);
  }

  if (!connect_) {
    tasks_.push_back([&]() { conn_->SendData(buff, len, true); });
  } else {
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        conn_, buff, len, true));
  }
}

void Client::SendData(const std::string &message) {
  int len = message.size();
  char *buff = (char *)malloc(len);
  memcpy(buff, const_cast<char *>(message.data()), len);
  if (!connect_) {
    tasks_.push_back(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        conn_, buff, len, true));
  } else {
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        conn_, buff, len, true));
  }
}

void Client::SendData(IOBuffer *buffer) {
  int len = buffer->GetReadAbleSize();
  char *buff = (char *)malloc(len);
  memcpy(buff, buffer->GetReadAblePtr(), len);
  if (!connect_) {
    tasks_.push_back(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        conn_, buff, len, true));
  } else {
    looper_->AddTask(std::bind(
        static_cast<void (TcpConnection::*)(const void *, size_t, bool)>(
            &TcpConnection::SendData),
        conn_, buff, len, true));
  }
}
