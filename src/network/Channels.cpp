//
// Created by cjw on 21-4-1.
//

#include "Channels.h"
#include "Dispatcher.h"
#include <sys/epoll.h>

Channels::Channels(Dispatcher::Ptr& loop,int fd) : disp_(loop),fd_(fd) ,events_(0) {}

Channels::~Channels() {}

void Channels::UpdateEventOption(Channels::EventOption event_option,
                                 uint32_t event_state) {
  event_option_ = event_option;
  events_ = event_state;
  disp_->UpdateChannels(this);
}

void Channels::HandleEvents(uint32_t events) {
  events_ = events;
  if (events_ & EPOLLIN) {
    if(HandleRead()<0){
      HandleError();
    }
  } else if (events_ & EPOLLOUT) {
    if(HandleWrite()<0){
      HandleError();
    }
  } else if (events_ & EPOLLERR) {
    HandleError();
  }
}

void Channels::EnableRead() {
  events_ |= EPOLLIN;
  UpdateEventOption(Channels::kEventAdd, events_);
}

void Channels::DisableRead() {
  events_ &= ~EPOLLIN;
  UpdateEventOption(Channels::kEventAdd, events_);
}

void Channels::EnableWrite() {
  events_ |= EPOLLOUT;
  UpdateEventOption(Channels::kEventAdd, events_);
}

void Channels::DisableWrite() {
  events_ &= ~EPOLLOUT;
  UpdateEventOption(Channels::kEventAdd, events_);
}