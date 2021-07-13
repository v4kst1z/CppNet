//
// Created by v4kst1z.
//

#ifndef CPPNET_BASE_SAFEQUEUE_H_
#define CPPNET_BASE_SAFEQUEUE_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "Common.h"

template <typename T>
class SafeQueue {
 public:
  SafeQueue() : head_(new node), tail(head_.get()), nums_(0) {}

  void Push(std::unique_ptr<T> &&new_value) {
    std::unique_ptr<node> new_node(new node);
    std::lock_guard<std::mutex> tail_lock(tail_mutex_);
    tail->data_ = std::move(new_value);
    tail->next_ = std::move(new_node);
    tail = tail->next_.get();
    {
      std::lock_guard<std::mutex> nums_lck(nums_mutex_);
      nums_++;
      if (nums_ == 1) data_cond_.notify_one();
    }
  }

  std::unique_ptr<T> WaitPop() {
    std::unique_lock<std::mutex> head_lock(head_mutex_);
    data_cond_.wait(head_lock, [this]() { return nums_ != 0; });
    std::unique_ptr<T> result(std::move(head_->data_));
    head_ = std::move(head_->next_);
    {
      std::lock_guard<std::mutex> nums_lck(nums_mutex_);
      nums_--;
    }
    return result;
  }

  bool Empty() const {
    std::lock(head_mutex_, tail_mutex_);
    std::lock_guard<std::mutex> headLk(head_mutex_, std::adopt_lock);
    std::lock_guard<std::mutex> tailLk(tail_mutex_, std::adopt_lock);
    return head_.get() == tail;
  }

  DISALLOW_COPY_AND_ASSIGN(SafeQueue);

 private:
  struct node {
    std::unique_ptr<T> data_;
    std::unique_ptr<node> next_;
  };

  std::unique_ptr<node> head_;
  mutable std::mutex head_mutex_;
  node *tail;
  mutable std::mutex tail_mutex_;
  std::condition_variable data_cond_;
  mutable std::mutex nums_mutex_;
  unsigned int nums_;
};

#endif  // CPPNET_BASE_SAFEQUEUE_H_
