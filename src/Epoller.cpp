//
// Created by v4kst1z
//

extern "C" {
#include <unistd.h>
}

#include "../include/Epoller.h"
#include "../include/Logger.h"

Epoller::Epoller(int time_out, int events_num)
    : epfd_(epoll_create1(EPOLL_CLOEXEC)),
      time_out_(time_out),
      events_num_(events_num) {
  events_ = (epoll_event *)malloc(events_num_ * sizeof(epoll_event));
}

void Epoller::AddEvent(std::shared_ptr<VariantEventBase> event_base) {
  struct epoll_event tep;
  event_base->Visit(
      [&tep](EventBase<Event> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      },
      [&tep](EventBase<TimeEvent> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      });
  DEBUG << "fd is " << tep.data.fd << " events is " << tep.events;
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, tep.data.fd, &tep) < 0)
    ERROR << "error epoll_ctl add";
  fd_to_events_.insert({tep.data.fd, event_base});
}

void Epoller::ModEvent(std::shared_ptr<VariantEventBase> event_base) {
  struct epoll_event tep;
  event_base->Visit(
      [&tep](EventBase<Event> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      },
      [&tep](EventBase<TimeEvent> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      });
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, tep.data.fd, &tep) < 0)
    ERROR << "error epoll_ctl";
}

void Epoller::DelEvent(std::shared_ptr<VariantEventBase> event_base) {
  struct epoll_event tep;
  event_base->Visit(
      [&tep](EventBase<Event> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      },
      [&tep](EventBase<TimeEvent> &e) {
        tep.events = e.GetEvents();
        tep.data.fd = e.GetFd();
      });
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, tep.data.fd, &tep) < 0)
    ERROR << "error epoll_ctl";
  fd_to_events_.erase(tep.data.fd);
}

std::vector<std::shared_ptr<VariantEventBase>> Epoller::PollWait() {
  int count = epoll_wait(epfd_, events_, events_num_, time_out_);
  if (count < 0) ERROR << "error epoll_wait " << errno;
  std::vector<std::shared_ptr<VariantEventBase>> ret_events;
  if (count) DEBUG << "epoll_wait count is " << count;
  for (int id = 0; id < count; id++) {
    auto eventbase =
        fd_to_events_[(events_ + id * sizeof(epoll_event))->data.fd];
    std::cout << "fd is " << (events_ + id * sizeof(epoll_event))->data.fd
              << std::endl;
    eventbase->Visit(
        [&](EventBase<Event> &e) {
          e.SetRevents((events_ + id * sizeof(epoll_event))->events);
        },
        [&](EventBase<TimeEvent> &e) {
          e.SetRevents((events_ + id * sizeof(epoll_event))->events);
        });
    ret_events.push_back(eventbase);
  }
  return ret_events;
}

Epoller::~Epoller() {
  free(events_);
  close(epfd_);
}

std::shared_ptr<VariantEventBase> Epoller::GetEventPtr(int fd) {
  return fd_to_events_[fd];
}
