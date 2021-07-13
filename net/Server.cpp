//
// Created by v4kst1z
//

#include "Server.h"

#include <csignal>
#include <memory>

#include "Ipv4Addr.h"
#include "Logger.h"
#include "TimerManager.h"

Server::Server(int io_threads_num, int timer_num, unsigned short port,
               uint8_t tpool_num)
    : server_port_(port),
      server_addr_(new Ipv4Addr(server_port_)),
      timer_manager_(std::make_shared<TimerManager>()),
      main_thread_(new Looper<TcpConnection>(server_addr_)),
      tpool_(new ThreadPool(tpool_num)),
      log_(Logger::GetInstance()) {
  signal(SIGPIPE, SIG_IGN);
  main_thread_->SetLoopId(std::this_thread::get_id());
  if (tpool_->GetThreadNum()) main_thread_->SetTPollPtr(tpool_.get());
  for (int id = 0; id < timer_num; id++) {
    auto looper = new Looper<TcpConnection>(timer_manager_, server_addr_);
    if (tpool_->GetThreadNum()) looper->SetTPollPtr(tpool_.get());
    io_threads_.push_back(looper);
  }
  for (int id = timer_num; id < io_threads_num; id++) {
    auto looper = new Looper<TcpConnection>(server_addr_);
    if (tpool_->GetThreadNum()) looper->SetTPollPtr(tpool_.get());
    io_threads_.push_back(looper);
  }
  log_.Start();
}

void Server::SetNewConnCallback(TcpConnection::CallBack &&cb) {
  new_conn_callback_ = std::move(cb);
  main_thread_->SetNewConnCallback(new_conn_callback_);
  for (auto &io : io_threads_) io->SetNewConnCallback(new_conn_callback_);
}

void Server::SetMessageCallBack(TcpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
  main_thread_->SetMessageCallBack(message_callback_);
  for (auto &io : io_threads_) io->SetMessageCallBack(message_callback_);
}

void Server::SetCloseCallBack(TcpConnection::CallBack &&cb) {
  close_callback_ = std::move(cb);
  main_thread_->SetCloseCallBack(close_callback_);
  for (auto &io : io_threads_) io->SetCloseCallBack(close_callback_);
}

void Server::SetSendDataCallBack(TcpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
  main_thread_->SetSendDataCallBack(send_data_callback_);
  for (auto &io : io_threads_) io->SetSendDataCallBack(send_data_callback_);
}

void Server::SetErrorCallBack(TcpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
  main_thread_->SetErrorCallBack(error_callback_);
  for (auto &io : io_threads_) io->SetErrorCallBack(error_callback_);
}

void Server::LoopStart() {
  for (auto &io : io_threads_) {
    io->SetLoopFlag(LOOPFLAG::SERVER);
    io->Start();
  }
  main_thread_->SetLoopFlag(LOOPFLAG::SERVER);
  main_thread_->Loop();
}

void Server::AddTimer(int timeout, std::function<void()> fun) {
  timer_manager_->AddTimer(timeout, fun);
}

void Server::Exit() {
  for (auto &io : io_threads_) io->Stop();
  for (auto &io : io_threads_) delete io;
  delete main_thread_;
  delete server_addr_;

  timer_manager_->Stop();
  log_.Stop();
}

Server::~Server() { Exit(); }

ThreadPool *Server::GetThreadPoolPtr() { return tpool_.get(); }
