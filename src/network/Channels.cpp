//
// Created by cjw on 21-4-1.
//
#include "Channels.h"
#include "Dispatcher.h"
#include "TcpServer.h"
#include <atomic>
#include <glog/logging.h>

Channels::Channels(Dispatcher::Ptr &loop, int fd)
    : disp_(loop), fd_(fd), events_(0) {}

Channels::~Channels() {}

// std::string &Channels::GetType() { return class_name; }

void Channels::UpdateEventOption(Channels::EventOption event_option,
                                 uint32_t event_state) {
  event_option_ = event_option;
  events_ = event_state;
  disp_->UpdateChannels(this);
}

void Channels::SetEvents(uint32_t events) { events_ = events; }

void Channels::EnableRead() {
  events_ |= EPOLLIN;
  UpdateEventOption(kEventMod, events_);
}

void Channels::DisableRead() {
  events_ &= ~EPOLLIN;
  UpdateEventOption(kEventMod, events_);
}

void Channels::EnableWrite() {
  events_ |= EPOLLOUT;
  UpdateEventOption(kEventMod, events_);
}


