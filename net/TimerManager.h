//
// Created by v4kst1z
//

#ifndef CPPNET_TIMERMANAGER_H
#define CPPNET_TIMERMANAGER_H

#include <unordered_map>
#include <thread>
#include <queue>
#include <vector>
#include <condition_variable>
#include <functional>

#include <Common.h>
#include <PriorityQueue.h>

class TimerManager;

class Timer {
 public:
  Timer(unsigned long long timeout, std::function<void()> callback, bool once = true) :
      time_out_(timeout),
      callback_(std::move(callback)),
      once_(once) {
    expire_ = GetCurrentMillisecs() + timeout;
  }

  unsigned long long GetExpire() {
    return expire_;
  }

  unsigned long long GetTimeOut() {
    return time_out_;
  }

  unsigned long long GetCurrentMillisecs() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / (1000 * 1000);
  }

  std::function<void()> GetCallBack() {
    return callback_;
  }

  bool GetOnceFlag() {
    return once_;
  }

  void Run() {
    callback_();
  }
 private:
  std::function<void()> callback_;
  unsigned long long time_out_;
  unsigned long long expire_;
  bool once_;
};

class TimerManager {
 public:
  using TimerPtr = std::shared_ptr<Timer>;
  using map_value_type = std::unordered_map<int, bool>::value_type;

  TimerManager();

  void AddTimer(int timeout, std::function<void()> fun);
  void DelTimer(std::shared_ptr<Timer> timer);
  void ResetTimerFd(std::shared_ptr<Timer>, int timer_fd_);
  void GetTimeSpec(struct timespec *ts, unsigned long long millisec);
  void HandelTimeout(int fd = 0);
  void SetFdFlag(int fd);
  void SetTimer();
  void Start();
  void Loop();
  void Stop();

  DISALLOW_COPY_AND_ASSIGN(TimerManager);
 private:

  unsigned long long GetCurrentMillisecs();

  struct cmp {
    bool operator()(const TimerPtr &lhs, const TimerPtr &rhs) {
      return lhs->GetExpire() > rhs->GetExpire();
    }
  };

  bool quit_;
  std::thread time_manager_thread_;
  std::unordered_map<int, bool> fd_timer_flag_;
  std::unordered_map<int, TimerPtr> fd_to_timer_;
  std::mutex mut_;
  std::condition_variable con_;
  PriorityQueue<TimerPtr, std::vector<TimerPtr>, cmp> timer_queue_;
};

#endif //CPPNET_TIMERMANAGER_H

