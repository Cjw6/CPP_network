#include "Dispatcher.h"
#include "util/Thread.h"
#include "network/Channels.h"
#include <glog/logging.h>
#include <mutex>

Dispatcher::Dispatcher() : epoll_fd_(-1), thread_id_(-1), running_flag_(0) {}

Dispatcher::~Dispatcher() {}

bool Dispatcher::InitLoop(int epoll_num) {
  epoll_lists_.resize(epoll_num);
  epoll_fd_ = epoll_create(epoll_num);
  if (epoll_fd_ < 0) {
    LOG(ERROR) << "create epoll fail ...";
    return false;
  }
  return true;
}

void Dispatcher::Dispatch() {
  thread_id_ = GetThreadId();
  for (auto task : task_exec_queue_) {
    task();
  }
  for (auto task : task_wait_queue_) {
    task();
  }

  running_flag_ = 1;
  while (running_flag_) {
    int event_count =
        epoll_wait(epoll_fd_, &epoll_lists_[0], epoll_lists_.size(), -1);
    if (event_count < 0) {
      if (errno == EINTR)
        return;
      else
        LOG(FATAL) << "event count" << event_count
                   << " error:" << strerror(errno);
    }

    for (int i = 0; i < event_count; i++) {
      CHECK(epoll_lists_[i].data.ptr);
      Channels *chann = reinterpret_cast<Channels *>(epoll_lists_[i].data.ptr);
      CHECK(chann);
      chann->HandleEvents(epoll_lists_[i].events);
    }
  }
}

void Dispatcher::AddTask(TaskCb task) {
  if (thread_id_ == GetThreadId()) {
    task();
  }
  std::lock_guard<std::mutex> lg(exec_queue_mutex_);
  task_wait_queue_.emplace_back(task);
}

void Dispatcher::AddEvent(int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void Dispatcher::DeleteEvent(int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
}

void Dispatcher::ModifyEvent(int fd, int state) {
  struct epoll_event ev;
  ev.events = state;
  ev.data.fd = fd;
  epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void Dispatcher::UpdateChannels(Channels *pc)
{
    DCHECK_NOTNULL(pc);

    int epoll_op   = -1;
    epoll_event ev = {0};
    ev.data.ptr    = pc;
    ev.events      = pc->GetEvents();

    switch (pc->GetEventOption())
    {
        case Channels::kEventAdd:
            epoll_op = EPOLL_CTL_ADD;
            break;
        case Channels::kEventMod:
            epoll_op = EPOLL_CTL_MOD;
            break;
        case Channels::kEventDel:
            epoll_op = EPOLL_CTL_DEL;
            break;
        default:
            LOG(ERROR) << "epoll option error";
            return;
    }
    epoll_ctl(epoll_fd_, epoll_op, pc->GetFd(), &ev);
}


