#include "network/WakeUp.h"
#include "network/Dispatcher.h"
#include "util/Log.h"

#include <errno.h>
#include <sys/eventfd.h>

WakeChannel::WakeChannel(DispatcherPtr &loop) : Channels(loop), event_fd_(-1) {}

WakeChannel::~WakeChannel() {
  if (event_fd_ > 0) {
    close(event_fd_);
  }
}

int WakeChannel::InitEventFd() {
  event_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd_ < 0) {
    LOG(INFO) << "eventfd fail";
    return -1;
  }
  disp_->UpdateChannels(this, event_fd_, kEventAdd, EPOLLIN);
  return 1;
}

void WakeChannel::HandleEvents() {
  if (events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (HandleRead() <= 0) {
      return;
    }
  }
}

int WakeChannel::WakeUp() {
  uint64_t one = 1;
  ssize_t n = write(event_fd_, &one, sizeof one);
  if (n != sizeof one) {
    PLOG(ERROR) << "WakeChannel::WakeUp error";
    return -1;
  }
  return 1;
}

int WakeChannel::HandleRead() {
  uint64_t one = 1;
  ssize_t n = 0;
  // 如果有多次的唤醒  统一处理 
  do {
    n = read(event_fd_, &one, sizeof one);
    if (n != sizeof one) {
      if (errno == EAGAIN) {
        break;
      }
      PLOG(ERROR) << "WakeChannel::HandleRead error";
      return -1;
    }
  } while (n > 0);
  return 1;
}