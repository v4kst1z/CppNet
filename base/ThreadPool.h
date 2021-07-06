//
// Created by v4kst1z
//

#ifndef CPPNET_THREADPOOL_H
#define CPPNET_THREADPOOL_H

#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <future>
#include <vector>

#include <Common.h>

class ThreadPool {
 public:
  explicit ThreadPool(uint8_t num_threads);

  template<typename Func, typename... Args>
  auto enqueue(Func &&f, Args &&... args) -> std::future<decltype(std::forward<Func>(f)(std::forward<Args>(args)...))>;

  uint8_t GetThreadNum();

  ~ThreadPool();

  DISALLOW_COPY_AND_ASSIGN(ThreadPool);
 private:
  class BaseWork {
   public:
    virtual ~BaseWork() = default;
    virtual void operator()() = 0;
  };

  template<typename RT>
  class Work : public BaseWork {
   public:
    explicit Work(std::packaged_task<RT()> func) : func_(std::move(func)) {}

    void operator()() override {
      func_();
    }
   private:
    std::packaged_task<RT()> func_;
  };

  void ThreadManager();

  std::condition_variable tp_cond_;
  std::queue<std::unique_ptr<BaseWork>> tasks_;
  std::mutex tasks_mtx_;
  std::vector<std::thread> thread_pool_;
  std::atomic<bool> stop_;
  int num_threads_;
};

inline ThreadPool::ThreadPool(uint8_t num_threads) :
    num_threads_(num_threads),
    stop_(false) {
  const uint8_t kMAX_THREADS = std::thread::hardware_concurrency() - 1;
  if (num_threads_ > kMAX_THREADS)
    num_threads_ = kMAX_THREADS;
  stop_.store(false);
  thread_pool_.reserve(num_threads_);
  for (uint8_t id = 0; id < num_threads_; id++) {
    thread_pool_.emplace_back(std::thread(&ThreadPool::ThreadManager, this));
  }
}

template<typename Func, typename ...Args>
inline auto ThreadPool::enqueue(Func &&f, Args &&...args) -> std::future<decltype(std::forward<Func>(f)(std::forward<
    Args>(args)...))> {
  using ret_type = decltype(std::forward<Func>(f)(std::forward<Args>(args)...));
  std::function<ret_type()> func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
  std::packaged_task<ret_type()> tsk = std::packaged_task<ret_type()>(func);
  std::future<ret_type> ret_future = tsk.get_future();
  {
    std::unique_lock<std::mutex> lck(tasks_mtx_);
    tasks_.emplace(std::unique_ptr<BaseWork>(new Work<ret_type>(std::move(tsk))));
  }
  tp_cond_.notify_one();
  return ret_future;
}

inline void ThreadPool::ThreadManager() {
  for (;;) {
    std::unique_ptr<BaseWork> tsk;
    {
      std::unique_lock<std::mutex> lck(tasks_mtx_);
      tp_cond_.wait(lck, [this]() { return stop_ || !tasks_.empty(); });
      if (stop_ && tasks_.empty()) return;
      tsk = std::move(tasks_.front());
      tasks_.pop();
      (*tsk)();
    }
  }
}

inline uint8_t ThreadPool::GetThreadNum() {
  return num_threads_;
}

inline ThreadPool::~ThreadPool() {
  stop_.store(true);
  tp_cond_.notify_all();
  for (auto &t : thread_pool_)
    t.join();
}

#endif //CPPNET_THREADPOOL_H
