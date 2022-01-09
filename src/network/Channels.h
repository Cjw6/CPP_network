//
// Created by cjw on 21-4-1.
//

#ifndef SERVERPROJECT_CHANNELS_H
#define SERVERPROJECT_CHANNELS_H

#include "Dispatcher.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <sys/epoll.h>

class Channels {
  friend class Dispatcher;

public:
  enum EventOption {
    kEventAdd = EPOLL_CTL_ADD,
    kEventMod = EPOLL_CTL_MOD,
    kEventDel = EPOLL_CTL_DEL
  };

  explicit Channels(Dispatcher::Ptr &loop, int fd);
  virtual ~Channels();

  // virtual std::string &GetType();
  // virtual int HandleRead();
  // virtual int HandleWrite();
  // virtual int HandleError();

  void SetEvents(uint32_t events);
  virtual void HandleEvents()=0;

  void UpdateEventOption(EventOption event_option, uint32_t event_state);

  void EnableRead();
  void DisableRead();
  void EnableWrite();
  void DisableWrite();

  EventOption GetEventOption() { return event_option_; }
  uint32_t GetEvents() { return events_; }
  int GetFd() { return fd_; }

protected:
  int fd_;
  EventOption event_option_;
  uint32_t events_;
  Dispatcher::Ptr disp_;
};

// void HandleEvents(Channels* p);

#endif // SERVERPROJECT_CHANNELS_H
