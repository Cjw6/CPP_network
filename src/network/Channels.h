//
// Created by cjw on 21-4-1.
//

#ifndef SERVERPROJECT_CHANNELS_H
#define SERVERPROJECT_CHANNELS_H

// #include "Dispatcher.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <sys/epoll.h>

enum EventOption : uint32_t {
  kEventAdd = EPOLL_CTL_ADD,
  kEventMod = EPOLL_CTL_MOD,
  kEventDel = EPOLL_CTL_DEL
};

class Dispatcher;
using DispatcherPtr = std::shared_ptr<Dispatcher>;

class Channels {
  friend class Dispatcher;

public:
  explicit Channels(DispatcherPtr &loop);
  // explicit Channels(Dispatcher::Ptr &loop, int fd);
  virtual ~Channels();

  void UpdateEventOption(int fd, EventOption event_option,
                         uint32_t event_state);
  void SetEvents(uint32_t events) { events_ = events; }

  void ResetDispatcher(DispatcherPtr &loop);

  virtual void HandleEvents() = 0;
  virtual int GetFd() = 0;

  // virtual std::string &GetType();
  // virtual int HandleRead();
  // virtual int HandleWrite();
  // virtual int HandleError();

  // void EnableRead();
  // void DisableRead();
  // void EnableWrite();
  // void DisableWrite();

  // EventOption GetEventOption() { return event_option_; }
  // uint32_t GetEvents() { return events_; }
  // void SetFd(int fd) { fd_ = fd; }
  // int GetFd() { return fd_; }

protected:
  // int fd_;
  // EventOption event_option_;
  uint32_t events_;
  DispatcherPtr disp_;
};

// void HandleEvents(Channels* p);

#endif // SERVERPROJECT_CHANNELS_H
