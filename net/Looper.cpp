//
// Created by v4kst1z
//

extern "C" {
#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/eventfd.h>
}

#include <TimerManager.h>
#include <Event.h>
#include <Epoller.h>
#include <Acceptor.h>
#include <Looper.h>

int CreateEventFd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    ERROR << "eventfd failed";
    exit(1);
  }
  return evtfd;
}

Looper::Looper(Ipv4Addr *addr, bool io_thread_flag) :
    quit_(false),
    epoller_(new Epoller()),
    net_addr_(addr),
    wakeup_fd_(CreateEventFd()) {
  EventBase<Event> fd = EventBase<Event>(wakeup_fd_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback([this]() {
    uint64_t num = 1;
    ssize_t n = read(wakeup_fd_, &num, sizeof(num));
  });
  epoller_->AddEvent(std::make_shared<VariantEventBase>(fd));
}

Looper::Looper(std::shared_ptr<TimerManager> timer_manager, Ipv4Addr *addr) :
    quit_(false),
    epoller_(new Epoller()),
    timer_manager_(timer_manager),
    net_addr_(addr),
    wakeup_fd_(CreateEventFd()),
    timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {
  EventBase<Event> event_fd = EventBase<Event>(wakeup_fd_);
  event_fd.EnableReadEvents(true);
  event_fd.SetReadCallback([this]() {
    uint64_t num = 1;
    ssize_t n = read(wakeup_fd_, &num, sizeof(num));
  });
  epoller_->AddEvent(std::make_shared<VariantEventBase>(event_fd));

  EventBase<TimeEvent> fd = EventBase<TimeEvent>(timer_fd_);
  timer_manager_->SetFdFlag(timer_fd_);
  fd.EnableReadEvents(true);
  fd.SetReadCallback(std::bind(&TimerManager::HandelTimeout, timer_manager_.get(), timer_fd_));
  timer_event_ = std::make_shared<VariantEventBase>(VariantEventBase(fd));
  epoller_->AddEvent(timer_event_);
}

void Looper::AddEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->AddEvent(e);
}

void Looper::ModEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->ModEvent(e);
}

void Looper::DelEvent(std::shared_ptr<VariantEventBase> e) {
  epoller_->ModEvent(e);
}

void Looper::Start() {
  io_thread_ = std::thread(std::bind(&Looper::Loop, this));
}

void Looper::SetAcceptNewConnection() {
  accptor_->SetNewConnectionCallBack([this](std::shared_ptr<TcpConnection> conn) -> void {

    conn->SetNewConnCallback(new_conn_callback_);
    conn->SetCloseCallBack(close_callback_);
    conn->SetErrorCallBack(error_callback_);
    conn->SetSendDataCallBack(send_data_callback_);
    conn->SetMessageCallBack(message_callback_);

    AddEvent(std::make_shared<VariantEventBase>(conn->GetEvent()));
    conn->RunNewConnCallBack();
  });
}

void Looper::Loop() {
  loop_id_ = std::this_thread::get_id();
  accptor_ = make_unique<Acceptor>(net_addr_, this);
  SetAcceptNewConnection();
  accptor_->Listen();

  std::shared_ptr<VariantEventBase> tmp = std::make_shared<VariantEventBase>(accptor_->GetEvent());
  epoller_->AddEvent(tmp);

  while (!quit_) {
    std::vector<std::shared_ptr<VariantEventBase>> ret = epoller_->PollWait();
    for (auto &elem : ret) {
      elem->Visit(
          [](EventBase<Event> &e) {
            e.HandleEvent();
          },
          [](EventBase<TimeEvent> &e) {
            e.HandleEvent();
          }
      );
    }
    ExecTask();
  }
}

void Looper::Stop() {
  quit_ = true;
  if (io_thread_.joinable())
    io_thread_.join();
}

void Looper::SetNewConnCallback(const TcpConnection::CallBack &cb) {
  new_conn_callback_ = cb;
}

void Looper::SetMessageCallBack(const TcpConnection::MessageCallBack &cb) {
  message_callback_ = cb;
}

void Looper::SetCloseCallBack(const TcpConnection::CallBack &cb) {
  close_callback_ = cb;
}

void Looper::SetSendDataCallBack(const TcpConnection::CallBack &cb) {
  send_data_callback_ = cb;
}

void Looper::SetErrorCallBack(const TcpConnection::CallBack &cb) {
  error_callback_ = cb;
}

std::thread::id Looper::GetThreadId() const {
  return loop_id_;
}

void Looper::WakeUpLoop() {
  uint64_t num = 1;
  ssize_t n = write(wakeup_fd_, &num, sizeof(num));
}

Looper::~Looper() {
  Stop();
}

void Looper::AddTask(WorkFunction task) {
  {
    std::lock_guard<std::mutex> lck(mtx_);
    task_.push_back(task);
  }
  WakeUpLoop();
}

void Looper::ExecTask() {
  std::vector<WorkFunction> tsk;
  {
    std::lock_guard<std::mutex> lck(mtx_);
    tsk.swap(task_);
  }
  for (auto &func : tsk)
    func();
}

void Looper::InsertConn(int fd, std::shared_ptr<TcpConnection> conn) {
  fd_to_conn_.insert({fd, conn});
}

std::shared_ptr<VariantEventBase> Looper::GetEventPtr(int fd) {
  return epoller_->GetEventPtr(fd);
}

void Looper::EraseConn(int fd) {
  fd_to_conn_.erase(fd);
}

void Looper::SetTPollPtr(ThreadPool *tpool) {
  tpool_ = tpool;
}

ThreadPool *Looper::GetTPollPtr() {
  return tpool_;
}





