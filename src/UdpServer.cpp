//
// Created by v4kst1z
//

#include "../include/UdpServer.h"

#include <cstring>

#include "../include/Logger.h"
#include "../include/Looper.h"
#include "../include/ThreadPool.h"
#include "../include/UdpConnection.h"

UdpServer::UdpServer(int io_threads_num, int timer_num, unsigned short port,
                     uint8_t tpool_num)
    : quit_(false),
      server_port_(port),
      server_addr_(new Ipv4Addr(port)),
      timer_manager_(std::make_shared<TimerManager>()),
      main_thread_(new Looper<UdpConnection>(server_addr_)),
      tpool_(new ThreadPool(tpool_num)),
      log_(Logger::GetInstance()) {
  main_thread_->SetLoopId(std::this_thread::get_id());
  if (tpool_->GetThreadNum()) main_thread_->SetTPollPtr(tpool_.get());
  for (int id = 0; id < timer_num; id++) {
    auto looper = new Looper<UdpConnection>(timer_manager_, server_addr_);
    if (tpool_->GetThreadNum()) looper->SetTPollPtr(tpool_.get());
    io_threads_.push_back(looper);
  }
  for (int id = timer_num; id < io_threads_num; id++) {
    auto looper = new Looper<UdpConnection>(server_addr_);
    if (tpool_->GetThreadNum()) looper->SetTPollPtr(tpool_.get());
    io_threads_.push_back(looper);
  }
  log_.Start();
}

void UdpServer::SetMessageCallBack(const UdpConnection::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
  main_thread_->SetMessageCallBack(message_callback_);
  for (auto &io : io_threads_) io->SetMessageCallBack(message_callback_);
}

void UdpServer::SetSendDataCallBack(const UdpConnection::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
  main_thread_->SetSendDataCallBack(send_data_callback_);
  for (auto &io : io_threads_) io->SetSendDataCallBack(send_data_callback_);
}

void UdpServer::SetErrorCallBack(const UdpConnection::CallBack &&cb) {
  error_callback_ = std::move(cb);
  main_thread_->SetErrorCallBack(error_callback_);
  for (auto &io : io_threads_) io->SetErrorCallBack(error_callback_);
}

void UdpServer::AddTimer(int timeout, std::function<void()> fun) {
  timer_manager_->AddTimer(timeout, fun);
}

ThreadPool *UdpServer::GetThreadPoolPtr() { return tpool_.get(); }

void UdpServer::LoopStart() {
  for (auto &io : io_threads_) {
    io->SetLoopFlag(LOOPFLAG::UDPSERVER);
    io->Start();
  }
  main_thread_->SetLoopFlag(LOOPFLAG::UDPSERVER);
  main_thread_->Loop();
}

void UdpServer::Exit() {
  for (auto &io : io_threads_) io->Stop();
  for (auto &io : io_threads_) delete io;
  delete main_thread_;
  delete server_addr_;

  timer_manager_->Stop();
  log_.Stop();
}

UdpServer::~UdpServer() { Exit(); }
