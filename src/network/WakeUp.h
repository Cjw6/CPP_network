#pragma once

#include "Channels.h"

class WakeChannel : public Channels {
public:
  WakeChannel();
  ~WakeChannel();
  void HandleEvents() override;
  int GetFd() override { return event_fd_; }

private:
  int event_fd_;
};