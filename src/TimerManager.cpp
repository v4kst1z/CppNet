//
// Created by v4kst1z
//

extern "C" {
#include <sys/timerfd.h>
#include <unistd.h>
}

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>

#include "../include/Logger.h"
#include "../include/TimerManager.h"

TimerManager::TimerManager() : quit_(false) { Start(); }

void TimerManager::AddTimer(int timeout, std::function<void()> fun) {
  if (timeout < 0) return;
  std::unique_lock<std::mutex> lck(mut_);
  std::shared_ptr<Timer> timer = std::make_shared<Timer>(timeout, fun);
  for (auto &elem : fd_timer_flag_) {
    if (elem.second) {
      if (fd_to_timer_[elem.first]->GetExpire() > timer->GetExpire()) {
        ResetTimerFd(timer, elem.first);
        timer_queue_.push(fd_to_timer_[elem.first]);
        fd_to_timer_[elem.first] = timer;
        return;
      }
    }
  }
  timer_queue_.push(timer);
  con_.notify_one();
}

void TimerManager::DelTimer(std::shared_ptr<Timer> timer) {
  timer_queue_.erase(timer);
}

void TimerManager::ResetTimerFd(std::shared_ptr<Timer> timer, int timer_fd_) {
  struct timespec t;
  struct itimerspec new_value;
  unsigned long long tmp = timer->GetExpire() - GetCurrentMillisecs();

  if (tmp < 0) return;
  GetTimeSpec(&t, tmp);
  memset(&new_value, 0, sizeof(new_value));

  new_value.it_value = t;
  timerfd_settime(timer_fd_, 0, &new_value, NULL);
}

void TimerManager::GetTimeSpec(struct timespec *ts,
                               unsigned long long millisec) {
  ts->tv_sec = (time_t)(millisec / 1000);
  ts->tv_nsec = (millisec % 1000) * 1000000;
}

void TimerManager::HandelTimeout(int fd) {
  uint64_t tmp;
  ssize_t n = read(fd, &tmp, sizeof(uint64_t));
  if (n != sizeof(uint64_t)) {
    ERROR << "read error";
  }

  std::unique_lock<std::mutex> lck(mut_);
  if (fd_to_timer_[fd]) {
    fd_to_timer_[fd]->Run();
    fd_timer_flag_[fd] = false;
    fd_to_timer_.erase(fd);
  }

  unsigned long long now = GetCurrentMillisecs();
  while (!timer_queue_.empty() && timer_queue_.top()->GetExpire() < now) {
    timer_queue_.top()->Run();
    if (!timer_queue_.top()->GetOnceFlag())
      AddTimer(timer_queue_.top()->GetTimeOut(),
               timer_queue_.top()->GetCallBack());
    timer_queue_.pop();
  }

  con_.notify_one();
}

void TimerManager::SetFdFlag(int fd) { fd_timer_flag_[fd] = false; }

void TimerManager::Start() {
  time_manager_thread_ = std::thread(&TimerManager::Loop, this);
}

void TimerManager::Loop() {
  while (!quit_) {
    std::unique_lock<std::mutex> lck(mut_);
    con_.wait(lck, [this]() {
      return !timer_queue_.empty() &&
             fd_timer_flag_.end() != std::find_if(fd_timer_flag_.begin(),
                                                  fd_timer_flag_.end(),
                                                  [](const map_value_type &mp) {
                                                    return mp.second == false;
                                                  });
    });

    for (auto &mp : fd_timer_flag_) {
      if (mp.second == false) {
        if (!timer_queue_.empty()) {
          ResetTimerFd(timer_queue_.top(), mp.first);
          mp.second = true;
          fd_to_timer_.insert({mp.first, timer_queue_.top()});
          timer_queue_.pop();
        }
      }
    }
  }
}

void TimerManager::Stop() {
  quit_ = true;
  if (time_manager_thread_.joinable()) time_manager_thread_.join();
}

unsigned long long TimerManager::GetCurrentMillisecs() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1000 + t.tv_nsec / (1000 * 1000);
}

TimerManager::~TimerManager() { Stop(); }
