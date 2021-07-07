//
// Created by v4kst1z.
//

#include <memory>

#include <Client.h>
#include <Socket.h>
#include <TcpConnection.h>

Client::Client(Ipv4Addr *addr) :
    quit_(false),
    addr_(addr),
    looper_(new Looper(addr)),
    log_(Logger::GetInstance()),
    conn_fd_(sockets::CreateNonblockAndCloexecTcpSocket()),
    conn_(std::make_shared<TcpConnection>(conn_fd_, looper_.get(), addr_)) {
  //looper_->AddEvent(std::make_shared<VariantEventBase>(conn_->GetEvent()));
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

void Client::LoopStart() {
  looper_->LoopClient(addr_.get());
}


