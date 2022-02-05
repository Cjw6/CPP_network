//
// Created by cjw on 21-4-1.
//
#include "Channels.h"
#include "Dispatcher.h"
#include "TcpServer.h"
#include <atomic>
#include <glog/logging.h>

Channels::Channels(Dispatcher::Ptr &loop) : disp_(loop) {}

Channels::~Channels() {}

void Channels::UpdateEventOption(int fd, EventOption event_option,
                                 uint32_t event_state) {
  disp_->UpdateChannels(this, fd, event_option, event_state);
}

void Channels::ResetDispatcher(Dispatcher::Ptr &loop) {
  DCHECK(loop);
  disp_ = loop;
}
