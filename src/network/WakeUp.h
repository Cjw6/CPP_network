#pragma once

#include "Channels.h"

class WakeChannel : public Channels {
public:
  WakeChannel(DispatcherPtr &loop);
  ~WakeChannel();
  int InitEventFd();
  int WakeUp();

  void HandleEvents() override;
  int GetFd() override { return event_fd_; }

private:
  int HandleRead();  

  int event_fd_;
};