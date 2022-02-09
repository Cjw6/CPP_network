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

#define kEventAdd EPOLL_CTL_ADD
#define kEventMod EPOLL_CTL_MOD
#define kEventDel EPOLL_CTL_DEL

class Dispatcher;
using DispatcherPtr = std::shared_ptr<Dispatcher>;

class Channels {
  friend class Dispatcher;

public:
  explicit Channels(DispatcherPtr &loop);
  // explicit Channels(Dispatcher::Ptr &loop, int fd);
  virtual ~Channels();

  void UpdateEventOption(int fd, uint32_t event_option, uint32_t event_state);
  void SetEvents(uint32_t events) { events_ = events; }

  void ResetDispatcher(DispatcherPtr &loop);

  virtual void HandleEvents() = 0;
  virtual int GetFd() = 0;

protected:
  uint32_t events_;
  DispatcherPtr disp_;
};

#endif // SERVERPROJECT_CHANNELS_H
