//
// Created by v4kst1z
//

#ifndef CPPNET_EPOLLER_H
#define CPPNET_EPOLLER_H

#include <memory>
#include <vector>
#include <unordered_map>

#include <Common.h>
#include <Event.h>

class Epoller {
 public:
  using FdEventsMap = std::unordered_map<int, std::shared_ptr<VariantEventBase>>;

  explicit Epoller(int time_out = 5000, int events_num = 4096);

  ~Epoller();

  void AddEvent(std::shared_ptr<VariantEventBase>);
  void ModEvent(std::shared_ptr<VariantEventBase>);
  void DelEvent(std::shared_ptr<VariantEventBase>);

  std::shared_ptr<VariantEventBase> GetEventPtr(int);

  std::vector<std::shared_ptr<VariantEventBase>> PollWait();

  DISALLOW_COPY_AND_ASSIGN(Epoller);
 private:
  FdEventsMap fd_to_events_;
  epoll_event *events_;
  int epfd_;
  int time_out_;
  int events_num_;
};

#endif //CPPNET_EPOLLER_H


