//
// Created by v4kst1z
//

#include <memory>

#include <TimerManager.h>
#include <Server.h>
#include <Ipv4Addr.h>
#include <Logger.h>

Server::Server(int io_threads_num, int timer_num, unsigned short port) :
    server_port_(port),
    server_addr_(new Ipv4Addr(server_port_)),
    timer_manager_(std::make_shared<TimerManager>()),
    main_thread_(new Looper(server_addr_, false)),
    log_(Logger::GetInstance()) {
  signal(SIGPIPE, SIG_IGN);
  for (int id = 0; id < timer_num; id++) {
    io_threads_.push_back(new Looper(timer_manager_, server_addr_));
  }
  for (int id = timer_num; id < io_threads_num; id++) {
    io_threads_.push_back((new Looper(server_addr_)));
  }
  log_.Start();
}

void Server::SetNewConnCallback(TcpConnection::CallBack &&cb) {
  new_conn_callback_ = std::move(cb);
  main_thread_->SetNewConnCallback(new_conn_callback_);
  for (auto &io: io_threads_)
    io->SetNewConnCallback(new_conn_callback_);
}

void Server::SetMessageCallBack(TcpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
  main_thread_->SetMessageCallBack(message_callback_);
  for (auto &io: io_threads_)
    io->SetMessageCallBack(message_callback_);
}

void Server::SetCloseCallBack(TcpConnection::CallBack &&cb) {
  close_callback_ = std::move(cb);
  main_thread_->SetCloseCallBack(close_callback_);
  for (auto &io: io_threads_)
    io->SetCloseCallBack(close_callback_);
}

void Server::SetSendDataCallBack(TcpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
  main_thread_->SetSendDataCallBack(send_data_callback_);
  for (auto &io: io_threads_)
    io->SetSendDataCallBack(send_data_callback_);
}

void Server::SetErrorCallBack(TcpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
  main_thread_->SetErrorCallBack(error_callback_);
  for (auto &io: io_threads_)
    io->SetErrorCallBack(error_callback_);
}

void Server::LoopStart() {
  for (auto &io: io_threads_)
    io->Start();
  main_thread_->Loop();
}

void Server::AddTimer(int timeout, std::function<void()> fun) {
  timer_manager_->AddTimer(timeout, fun);
}

void Server::Exit() {
  for (auto &io: io_threads_)
    io->Stop();
  for (auto &io: io_threads_)
    delete io;
  delete main_thread_;
  delete server_addr_;

  timer_manager_->Stop();
  log_.Stop();
}

Server::~Server() {
  Exit();
}

