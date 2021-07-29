/*
 * @Author: V4kst1z (dcydane@gmail.com)
 * @Date: 2021-07-27 10:10:47
 * @LastEditors: V4kst1z
 * @Description:
 * @FilePath: /CppNet/include/Epoller.h
 */
//
// Created by v4kst1z
//

#ifndef CPPNET_NET_EPOLLER_H
#define CPPNET_NET_EPOLLER_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "Event.h"

class Epoller {
 public:
  using FdEventsMap =
      std::unordered_map<int, std::shared_ptr<VariantEventBase>>;

  explicit Epoller(int time_out = 5000, int events_num = 4096);

  void AddEvent(std::shared_ptr<VariantEventBase>);
  void ModEvent(std::shared_ptr<VariantEventBase>);
  void DelEvent(std::shared_ptr<VariantEventBase>);

  std::shared_ptr<VariantEventBase> GetEventPtr(int);

  std::vector<std::shared_ptr<VariantEventBase>> PollWait();

  ~Epoller();

  DISALLOW_COPY_AND_ASSIGN(Epoller);

 private:
  FdEventsMap fd_to_events_;
  epoll_event *events_;
  int epfd_;
  int time_out_;
  int events_num_;
  std::mutex mp_mtx_;
};

#endif  // CPPNET_NET_EPOLLER_H
