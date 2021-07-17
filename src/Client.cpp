//
// Created by v4kst1z
//

#include "../include/Client.h"

#include <memory>

#include "../include/Looper.h"
#include "../include/Socket.h"
#include "../include/TcpConnection.h"

Client::Client(Looper<TcpConnection> *looper, std::shared_ptr<Ipv4Addr> addr)
    : quit_(false),
      addr_(addr),
      looper_(looper),
      log_(Logger::GetInstance()),
      conn_fd_(sockets::CreateNonblockAndCloexecTcpSocket()),
      conn_(std::make_shared<TcpConnection>(conn_fd_, looper_, addr_)) {
  conn_->GetEvent().EnableWriteEvents(true);
  looper_->InsertConn(conn_fd_, conn_);
  looper_->SetLoopId(std::this_thread::get_id());
  log_.Start();
}

void Client::SetNewConnCallback(TcpConnection::CallBack &&cb) {
  new_conn_callback_ = std::move(cb);
  looper_->SetNewConnCallback(new_conn_callback_);
}

void Client::SetMessageCallBack(TcpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
  looper_->SetMessageCallBack(message_callback_);
}

void Client::SetCloseCallBack(TcpConnection::CallBack &&cb) {
  close_callback_ = std::move(cb);
  looper_->SetCloseCallBack(close_callback_);
}

void Client::SetSendDataCallBack(TcpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
  looper_->SetSendDataCallBack(send_data_callback_);
}

void Client::SetErrorCallBack(TcpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
  looper_->SetErrorCallBack(error_callback_);
}

void Client::LoopStart() { looper_->Loop(); }

void Client::Connect() {
  sockets::Connect(conn_fd_, addr_.get());

  conn_->SetMessageCallBack(
      [&](const std::shared_ptr<TcpConnection> &conn, IOBuffer &) {
        conn->SetNewConnCallback(new_conn_callback_);
        conn->SetCloseCallBack(close_callback_);
        conn->SetErrorCallBack(error_callback_);
        conn->SetSendDataCallBack(send_data_callback_);
        conn->SetMessageCallBack(message_callback_);

        conn->RunNewConnCallBack();
        auto event = looper_->GetEventPtr(conn->GetConnFd());
        event->Visit([](EventBase<Event> &conn_event_) {
          conn_event_.EnableWriteEvents(false);
        });

        if (!looper_->GetLoopStartValue()) {
          looper_->SetLoopStartValue(true);
          looper_->ExecTask();
        }
      });
  looper_->AddEvent(std::make_shared<VariantEventBase>(conn_->GetEvent()));
}

void Client::SendData(const void *data, size_t len) {
  looper_->AddTask(
      std::bind(static_cast<void (TcpConnection::*)(const void *, size_t)>(
                    &TcpConnection::SendData),
                conn_, data, len));
}

void Client::SendData(const std::string &message) {
  looper_->AddTask(
      std::bind(static_cast<void (TcpConnection::*)(const std::string &)>(
                    &TcpConnection::SendData),
                conn_, message));
}

void Client::SendData(IOBuffer *buffer) {
  looper_->AddTask(std::bind(static_cast<void (TcpConnection::*)(IOBuffer *)>(
                                 &TcpConnection::SendData),
                             conn_, buffer));
}
