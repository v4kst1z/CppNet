//
// Created by v4kst1z
//

#ifndef CPPNET_NET_LOOPER_H
#define CPPNET_NET_LOOPER_H

extern "C" {
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
}

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Acceptor.h"
#include "Epoller.h"
#include "Event.h"
#include "TcpConnection.h"
#include "TimerManager.h"

class Ipv4Addr;
class TimerManager;
class Epoller;
class Acceptor;
class Server;
class TcpConnection;
class ThreadPool;

enum class LOOPFLAG { NONE, SERVER, CLIENT, UDPSERVER, UDPCLIENT };

class BaseLooper {
 public:
  using WorkFunction = std::function<void()>;

  BaseLooper() = default;

  virtual ~BaseLooper() = default;

  virtual void AddEvent(std::shared_ptr<VariantEventBase>) { return; }

  virtual void ModEvent(std::shared_ptr<VariantEventBase>) { return; }

  virtual void DelEvent(std::shared_ptr<VariantEventBase>) { return; }

  virtual std::shared_ptr<VariantEventBase> GetEventPtr(int) { return nullptr; }

  virtual void InsertConn(int, std::shared_ptr<TcpConnection>) { return; }

  virtual void EraseConn(int) { return; }

  virtual void SetTPollPtr(ThreadPool *) { return; }

  virtual ThreadPool *GetTPollPtr() { return nullptr; }

  virtual std::thread::id GetThreadId() const { return std::thread::id(); }

  virtual void AddTask(WorkFunction) { return; }

  virtual void AddTasks(std::vector<WorkFunction> &) { return; }

  virtual int GetServerFd() { return 0; }

  virtual void SetServerFd(int) { return; }
};

template <typename T>
class Looper final : public BaseLooper {
 public:
  explicit Looper(Ipv4Addr * = nullptr);

  //至少有一个线程支持 timer 定时器
  explicit Looper(std::shared_ptr<TimerManager>, Ipv4Addr * = nullptr);

  void AddEvent(std::shared_ptr<VariantEventBase>) override;
  void ModEvent(std::shared_ptr<VariantEventBase>) override;
  void DelEvent(std::shared_ptr<VariantEventBase>) override;

  void Start();
  void Loop();
  void Stop();

  void WakeUpLoop();

  void AddTask(WorkFunction task) override;

  void AddTasks(std::vector<WorkFunction> &task) override;

  void ExecTask();

  void SetAcceptNewConnection();

  void SetNewConnCallback(const typename T::CallBack &cb);
  void SetMessageCallBack(const typename T::MessageCallBack &cb);
  void SetCloseCallBack(const typename T::CallBack &cb);
  void SetSendDataCallBack(const typename T::CallBack &cb);
  void SetErrorCallBack(const typename T::CallBack &cb);

  std::shared_ptr<VariantEventBase> GetEventPtr(int) override;

  void InsertConn(int, std::shared_ptr<TcpConnection>) override;
  void EraseConn(int) override;

  void SetTPollPtr(ThreadPool *tpool) override;
  ThreadPool *GetTPollPtr() override;

  std::thread::id GetThreadId() const override;

  void SetLoopId(std::thread::id loop_id);
  std::thread::id GetLoopId();

  void AddTimer(int timeout, std::function<void()> fun);

  int GetServerFd() override;
  void SetServerFd(int fd) override;

  void SetServerFdImpl(int fd);

  void SetLoopFlag(LOOPFLAG flag);
  ~Looper();

  DISALLOW_COPY_AND_ASSIGN(Looper);

 private:
  template <typename U = T>
  typename std::enable_if<std::is_same<U, TcpConnection>::value, void>::type
  InitServer();

  template <typename U = T>
  typename std::enable_if<std::is_same<U, UdpConnection>::value, void>::type
  InitServer();

  void InitUdpServer();

  Ipv4Addr *net_addr_;
  int server_fd_;

  int timer_fd_;
  bool quit_;
  int wakeup_fd_;
  LOOPFLAG loop_flag;
  std::unique_ptr<Epoller> epoller_;
  std::thread io_thread_;
  std::thread::id loop_id_;
  std::mutex mtx_;
  std::vector<WorkFunction> task_;
  Acceptor *accptor_;
  std::shared_ptr<TimerManager> timer_manager_;
  std::shared_ptr<VariantEventBase> timer_event_;
  std::unordered_map<int, std::shared_ptr<TcpConnection>> fd_to_conn_;
  ThreadPool *tpool_;

  typename T::CallBack new_conn_callback_;
  typename T::MessageCallBack message_callback_;
  typename T::CallBack send_data_callback_;
  typename T::CallBack close_callback_;
  typename T::CallBack error_callback_;
};

inline int CreateEventFd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    ERROR << "eventfd failed";
    exit(1);
  }
  return evtfd;
}

template <typename T>
inline Looper<T>::Looper(Ipv4Addr *addr)
    : quit_(false),
      epoller_(new Epoller()),
      net_addr_(addr),
      wakeup_fd_(CreateEventFd()),
      loop_flag(LOOPFLAG::NONE) {
  EventBase<Event> fd = Event(wakeup_fd_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback([this]() {
    uint64_t num = 1;
    ssize_t n = 0;
    while ((n = read(wakeup_fd_, &num, sizeof(num))) > 0) {
    }
  });
  epoller_->AddEvent(std::make_shared<VariantEventBase>(fd));
}

template <typename T>
inline Looper<T>::Looper(std::shared_ptr<TimerManager> timer_manager,
                         Ipv4Addr *addr)
    : quit_(false),
      epoller_(new Epoller()),
      timer_manager_(timer_manager),
      net_addr_(addr),
      wakeup_fd_(CreateEventFd()),
      loop_flag(LOOPFLAG::NONE),
      timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {
  EventBase<Event> event_fd = Event(wakeup_fd_);
  event_fd.EnableReadEvents(true);
  event_fd.SetReadCallback([this]() {
    uint64_t num = 1;
    ssize_t n = 0;
    while ((n = read(wakeup_fd_, &num, sizeof(num))) > 0) {
    }
  });
  epoller_->AddEvent(std::make_shared<VariantEventBase>(event_fd));

  EventBase<TimeEvent> fd = TimeEvent(timer_fd_);
  timer_manager_->SetFdFlag(timer_fd_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback(
      std::bind(&TimerManager::HandelTimeout, timer_manager_.get(), timer_fd_));
  timer_event_ = std::make_shared<VariantEventBase>(fd);
  epoller_->AddEvent(timer_event_);
}

template <typename T>
inline void Looper<T>::AddEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->AddEvent(e);
}

template <typename T>
inline void Looper<T>::SetLoopFlag(LOOPFLAG flag) {
  loop_flag = flag;
}

template <typename T>
inline void Looper<T>::ModEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->ModEvent(e);
}

template <typename T>
inline void Looper<T>::DelEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->DelEvent(e);
}

template <typename T>
inline void Looper<T>::Start() {
  io_thread_ = std::thread(std::bind(&Looper::Loop, this));
}

template <typename T>
inline int Looper<T>::GetServerFd() {
  return server_fd_;
}

template <typename T>
void Looper<T>::SetAcceptNewConnection() {
  accptor_->SetNewConnectionCallBack(
      [this](std::shared_ptr<TcpConnection> conn) -> void {
        conn->SetNewConnCallback(new_conn_callback_);
        conn->SetCloseCallBack(close_callback_);
        conn->SetErrorCallBack(error_callback_);
        conn->SetSendDataCallBack(send_data_callback_);
        conn->SetMessageCallBack(message_callback_);

        AddEvent(std::make_shared<VariantEventBase>(conn->GetEvent()));
        conn->RunNewConnCallBack();
      });
}

template <typename T>
template <typename U>
typename std::enable_if<std::is_same<U, TcpConnection>::value, void>::type
Looper<T>::InitServer() {
  accptor_ = new Acceptor(net_addr_, this);
  SetAcceptNewConnection();
  accptor_->Listen();

  epoller_->AddEvent(std::make_shared<VariantEventBase>(accptor_->GetEvent()));
}

template <typename T>
template <typename U>
typename std::enable_if<std::is_same<U, UdpConnection>::value, void>::type
Looper<T>::InitServer() {
  server_fd_ = sockets::CreateNonblockAndCloexecUdpSocket();
  sockets::SetReuseAddr(server_fd_);
  sockets::SetReusePort(server_fd_);
  sockets::Bind(server_fd_, net_addr_);

  EventBase<Event> fd = Event(server_fd_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback([this]() {
    std::vector<std::shared_ptr<UdpConnection>> bak;
    while (true) {
      int error = 0;
      auto conn =
          std::make_shared<UdpConnection>(dynamic_cast<BaseLooper *>(this));
      bak.push_back(conn);
      conn->SetMessageCallBack(message_callback_);
      conn->SetSendDataCallBack(send_data_callback_);
      int ret = sockets::RecvFrom(server_fd_, conn, error);
      if (ret < 0) {
        if (error != EAGAIN) {
          DEBUG << "RecvFrom Failed, errno num is " << errno;
        }
        return;
      } else if (ret > 0) {
        conn->RunMessageCallBack();
      }
    }
  });

  epoller_->AddEvent(std::make_shared<VariantEventBase>(fd));
}

template <typename T>
inline void Looper<T>::Loop() {
  loop_id_ = std::this_thread::get_id();

  switch (loop_flag) {
    case LOOPFLAG::SERVER:
    case LOOPFLAG::UDPSERVER:
      InitServer();
      break;
    case LOOPFLAG::CLIENT:
    case LOOPFLAG::UDPCLIENT:
      break;
    default:
      DEBUG << "Please Set loop flag!";
      break;
  }

  while (!quit_) {
    std::vector<std::shared_ptr<VariantEventBase>> ret = epoller_->PollWait();
    for (auto &elem : ret) {
      elem->Visit([](EventBase<Event> &e) { e.HandleEvent(); },
                  [](EventBase<TimeEvent> &e) { e.HandleEvent(); });
    }
    ExecTask();
  }
}

template <typename T>
inline void Looper<T>::Stop() {
  quit_ = true;
  if (io_thread_.joinable()) io_thread_.join();
}

template <typename T>
inline void Looper<T>::SetNewConnCallback(const typename T::CallBack &cb) {
  new_conn_callback_ = cb;
}

template <typename T>
inline void Looper<T>::SetMessageCallBack(
    const typename T::MessageCallBack &cb) {
  message_callback_ = cb;
}

template <typename T>
inline void Looper<T>::SetCloseCallBack(const typename T::CallBack &cb) {
  close_callback_ = cb;
}

template <typename T>
inline void Looper<T>::SetSendDataCallBack(const typename T::CallBack &cb) {
  send_data_callback_ = cb;
}

template <typename T>
inline void Looper<T>::SetErrorCallBack(const typename T::CallBack &cb) {
  error_callback_ = cb;
}

template <typename T>
inline std::thread::id Looper<T>::GetThreadId() const {
  return loop_id_;
}

template <typename T>
inline void Looper<T>::WakeUpLoop() {
  uint64_t num = 1;
  ssize_t n = write(wakeup_fd_, &num, sizeof(num));
}

template <typename T>
inline Looper<T>::~Looper() {
  Stop();
  delete accptor_;
}

template <typename T>
inline void Looper<T>::AddTask(WorkFunction task) {
  {
    std::lock_guard<std::mutex> lck(mtx_);
    task_.push_back(task);
  }
  WakeUpLoop();
}

template <typename T>
inline void Looper<T>::ExecTask() {
  std::vector<WorkFunction> tsk;
  {
    std::lock_guard<std::mutex> lck(mtx_);
    tsk.swap(task_);
  }
  for (auto &func : tsk) func();
}

template <typename T>
inline void Looper<T>::InsertConn(int fd, std::shared_ptr<TcpConnection> conn) {
  fd_to_conn_.insert({fd, conn});
}

template <typename T>
inline std::shared_ptr<VariantEventBase> Looper<T>::GetEventPtr(int fd) {
  return epoller_->GetEventPtr(fd);
}

template <typename T>
inline void Looper<T>::EraseConn(int fd) {
  fd_to_conn_.erase(fd);
}

template <typename T>
inline void Looper<T>::SetTPollPtr(ThreadPool *tpool) {
  tpool_ = tpool;
}

template <typename T>
inline ThreadPool *Looper<T>::GetTPollPtr() {
  return tpool_;
}

template <typename T>
inline void Looper<T>::SetLoopId(std::thread::id loop_id) {
  loop_id_ = loop_id;
}

template <typename T>
inline std::thread::id Looper<T>::GetLoopId() {
  return loop_id_;
}

template <typename T>
inline void Looper<T>::AddTimer(int timeout, std::function<void()> fun) {
  timer_manager_->AddTimer(timeout, fun);
}

template <typename T>
void Looper<T>::SetServerFd(int fd) {
  server_fd_ = fd;
}

template <typename T>
void Looper<T>::AddTasks(std::vector<WorkFunction> &task) {
  {
    std::lock_guard<std::mutex> lck(mtx_);
    task_.insert(task_.end(), task.begin(), task.end());
  }

  WakeUpLoop();
}

#endif  // CPPNET_NET_LOOPER_H
