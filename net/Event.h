//
// Created by v4kst1z
//

#ifndef CPPNET_EVENT_H
#define CPPNET_EVENT_H

extern "C" {
#include <sys/epoll.h>
}

#include <functional>
#include <atomic>
#include <iostream>

#include <Common.h>
#include <Variant.h>

enum EVENTVALUE {
  EPREAD = EPOLLIN | EPOLLPRI,
  EPWRITE = EPOLLOUT,
  EPERROR = EPOLLERR,
  EPSEVERR = EPOLLHUP,
  EPCLICLO = EPOLLRDHUP
};

class Event;
class TimeEvent;

template<typename T, bool = std::is_same<T, Event>::value>
class EventBase {
 public:
  using EventCallback = std::function<void()>;

  explicit EventBase(int fd) :
      fd_(fd),
      events_(0),
      revents_(0) {}

  void EnableReadEvents(bool flag) {
    if (flag)
      events_ |= EPOLLIN | EPOLLPRI | EPOLLET;
    else
      events_ |= ~(EPOLLIN | EPOLLPRI | EPOLLET);
  }

  void SetEvent(int event) {
    events_ |= event;
  }

  void SetReadCallback(EventCallback &&cb) {
    read_cb_ = cb;
  }

  void HandleEvent() {
    static_cast<T *>(this)->HandleEvent();
  }

  int GetFd() { return fd_; }

  void SetRevents(short event) { revents_ = event; }

  short GetEvents() { return events_; }

 protected:
  int fd_;
  short events_;
  short revents_;

  EventCallback read_cb_;
};

template<typename T>
class EventBase<T, true> {
 public:
  using EventCallback = std::function<void()>;

  explicit EventBase(int fd) :
      fd_(fd),
      events_(0),
      revents_(0) {}

  void EnableReadEvents(bool flag) {
    if (flag)
      events_ |= EPOLLIN | EPOLLPRI | EPOLLET;
    else
      events_ &= ~(EPOLLIN | EPOLLPRI | EPOLLET);
  }

  void SetEvent(int event) {
    events_ |= event;
  }

  void SetReadCallback(EventCallback &&cb) {
    read_cb_ = cb;
  }

  void EnableWriteEvents(bool flag) {
    static_cast<T *>(this)->EnableWriteEvents(flag);
  }

  void SetCloseCallback(EventCallback &&cb) {
    static_cast<T *>(this)->SetCloseCallback(std::forward<EventCallback>(cb));
  }

  void SetWriteCallback(EventCallback &&cb) {
    static_cast<T *>(this)->SetWriteCallback(std::forward<EventCallback>(cb));
  }

  void SetErrorCallback(EventCallback &&cb) {
    static_cast<T *>(this)->SetErrorCallback(std::forward<EventCallback>(cb));
  }

  void HandleEvent() {
    static_cast<T *>(this)->HandleEvent();
  }

  int GetFd() { return fd_; }

  void SetRevents(short event) { revents_ = event; }

  short GetEvents() { return events_; }

 protected:
  int fd_;
  short events_;
  short revents_;

  EventCallback read_cb_;
};

using VariantEventBase = Variant<EventBase<TimeEvent>, EventBase<Event>>;

class TimeEvent : public EventBase<TimeEvent> {
 public:
  explicit TimeEvent(int fd) :
      EventBase(fd) {}

  void HandleEvent() {
    read_cb_();
  }

  ~TimeEvent() {}
};

class Event : public EventBase<Event> {
 public:
  explicit Event(int fd) : EventBase(fd) {}

  void EnableWriteEvents(bool flag) {
    if (flag)
      events_ |= EPOLLOUT;
    else
      events_ &= ~EPOLLOUT;
  }

  void SetWriteCallback(EventCallback &&cb) {
    write_cb_ = cb;
  }

  void SetErrorCallback(EventCallback &&cb) {
    error_cb_ = cb;
  }

  void SetCloseCallback(EventCallback &&cb) {
    close_cb_ = cb;
  }

  void HandleEvent() {
    if (((events_ & EPSEVERR) || (events_ & EPCLICLO)) && close_cb_) {
      close_cb_();
      return;
    }
    if ((events_ & EPERROR) && error_cb_) {
      error_cb_();
      return;
    }
    if ((events_ & EPREAD) && read_cb_)
      read_cb_();
    if ((events_ & EPWRITE) && write_cb_)
      write_cb_();
  }

  ~Event() {}
 private:
  EventCallback write_cb_;
  EventCallback error_cb_;
  EventCallback close_cb_;
};

#endif //CPPNET_EVENT_H


