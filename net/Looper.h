//
// Created by v4kst1z
//

#ifndef CPPNET_LOOPER_H
#define CPPNET_LOOPER_H

extern "C" {
#include <netinet/in.h>
}

#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>

#include <TcpConnection.h>

class Ipv4Addr;
class TimerManager;
class Epoller;
class Acceptor;
class Server;
class ThreadPool;

class Looper {
 public:
  using WorkFunction = std::function<void()>;

  explicit Looper(Ipv4Addr *, bool io_thread_flag = true);

  //至少有一个线程支持 timer 定时器
  Looper(std::shared_ptr<TimerManager>, Ipv4Addr *);

  void AddEvent(std::shared_ptr<VariantEventBase>);
  void ModEvent(std::shared_ptr<VariantEventBase>);
  void DelEvent(std::shared_ptr<VariantEventBase>);

  void Start();
  void Loop();
  void Stop();
  void WakeUpLoop();
  void AddTask(WorkFunction task);
  void ExecTask();

  void SetAcceptNewConnection();

  void SetNewConnCallback(const TcpConnection::CallBack &cb);
  void SetMessageCallBack(const TcpConnection::MessageCallBack &cb);
  void SetCloseCallBack(const TcpConnection::CallBack &cb);
  void SetSendDataCallBack(const TcpConnection::CallBack &cb);
  void SetErrorCallBack(const TcpConnection::CallBack &cb);

  std::shared_ptr<VariantEventBase> GetEventPtr(int);

  void InsertConn(int, std::shared_ptr<TcpConnection>);
  void EraseConn(int);

  void SetTPollPtr(ThreadPool *tpool);
  ThreadPool *GetTPollPtr();

  std::thread::id GetThreadId() const;

  ~Looper();

  DISALLOW_COPY_AND_ASSIGN(Looper);
 private:
  Ipv4Addr *net_addr_;

  int timer_fd_;
  bool quit_;
  int wakeup_fd_;
  std::unique_ptr<Epoller> epoller_;
  std::thread io_thread_;
  std::thread::id loop_id_;
  std::mutex mtx_;
  std::vector<WorkFunction> task_;
  std::unique_ptr<Acceptor> accptor_;
  std::shared_ptr<TimerManager> timer_manager_;
  std::shared_ptr<VariantEventBase> timer_event_;
  std::unordered_map<int, std::shared_ptr<TcpConnection>> fd_to_conn_;
  ThreadPool *tpool_;

  TcpConnection::CallBack new_conn_callback_;
  TcpConnection::MessageCallBack message_callback_;
  TcpConnection::CallBack send_data_callback_;
  TcpConnection::CallBack close_callback_;
  TcpConnection::CallBack error_callback_;
};

#endif //CPPNET_LOOPER_H


