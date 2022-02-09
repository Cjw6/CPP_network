#include "network/Dispatcher.h"
#include "network/Channels.h"
#include "network/WakeUp.h"

#include "util/Thread.h"
#include "util/ThreadPool.h"

#include "util/Log.h"

#include <functional>
#include <memory>
#include <mutex>
#include <sys/epoll.h>

static constexpr unsigned int str2int(const char *str, int h = 0) {
  return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

Dispatcher::Config::Config() : thread_num(1), epoll_max_listen(1000) {}

Dispatcher::Dispatcher()
    : epoll_fd_(-1), thread_id_(-1), running_flag_(0), thread_num_(0),
      epoll_max_listen_num_(0) {}

Dispatcher::~Dispatcher() {}

bool Dispatcher::InitLoop(Config &conf) {
  thread_num_ = conf.thread_num;
  epoll_max_listen_num_ = conf.epoll_max_listen;

  CHECK(thread_num_ > 0);
  CHECK(epoll_max_listen_num_ > 10);

  if (thread_num_ == 1) {
    work_mode_ = kSTMode;
    LOG(INFO) << "cur work mode: ST";
  } else {
    work_mode_ = kMTMode;
    pool_ = std::make_unique<ThreadPool>(thread_num_);
    LOG(INFO) << "cur work mode MT";
  }

  epoll_lists_.resize(epoll_max_listen_num_);
  epoll_fd_ = epoll_create(epoll_max_listen_num_);
  if (epoll_fd_ < 0) {
    LOG(ERROR) << "create epoll fail ...";
    return false;
  }

  auto self = shared_from_this();
  timer_ctl_ = std::make_unique<TimerController>();
  wake_ = std::make_unique<WakeChannel>(self);
  if (wake_->InitEventFd() < 0) {
    return false;
  }
  return true;
}

void Dispatcher::Dispatch() {
  thread_id_ = GetThreadId();
  {
    std::lock_guard<std::mutex> lg(exec_queue_mutex_);
    for (auto &task : task_exec_queue_) {
      task();
    }
    task_exec_queue_.clear();
    for (auto &task : task_wait_queue_) {
      task();
    }
    task_wait_queue_.clear();
  }

  running_flag_ = 1;
  int interval = timer_ctl_->CalIntervalMs();
  LOG(INFO) << "fisrt interval " << interval;
  int event_count = 0;
  int64_t loop_cnt = 0;
  // LOG(INFO) << running_flag_.load();
  while (running_flag_) {
    event_count =
        epoll_wait(epoll_fd_, &epoll_lists_[0], epoll_lists_.size(), interval);

    if (event_count < 0) {
      PLOG(ERROR) << "epoll wait:";
      if (errno == EINTR) {

      } else {
        LOG(ERROR) << "dispatcher quit\n";
        return;
      }
    }

    timer_ctl_->Schedule();
    HandlelAsyncQueue();
    ChannelHandelEvent(event_count);

    interval = timer_ctl_->CalIntervalMs();
    if (interval < 0) {
      interval = 13;
    } else {
      interval = std::min(interval, 13);
    }

    loop_cnt++;
    // LOG(INFO) << "print loop_cnt" << loop_cnt << " interval" << interval;
  }
  LOG(INFO) << "quit";
}

void Dispatcher::RunTask(const TaskCb &task, bool use_threadpool) {
  if (work_mode_ == kMTMode && use_threadpool) {
    pool_->enqueue(std::move(task));
  } else if (thread_id_ == GetThreadId()) {
    task();
  } else {
    {
      std::lock_guard<std::mutex> lg(exec_queue_mutex_);
      task_wait_queue_.emplace_back(std::move(task));
    }
    wake_->WakeUp();
  }
}

TimerTask::Id Dispatcher::AddTimerTask(TimerTask::Func func, TimeUs delay,
                                       bool singleshot) {
  if (!timer_ctl_) {
    return -1;
  }

  return timer_ctl_->AddTask(std::move(func), delay, singleshot);
}

void Dispatcher::RemoveTimerTask(TimerTask::Id &id) {
  if (!timer_ctl_) {
    id = -1;
    return;
  }

  timer_ctl_->RemoveTimerTask(id);
}

void Dispatcher::Wake() {
  if (!wake_) {
    return;
  }
  wake_->WakeUp();
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

void Dispatcher::UpdateChannels(Channels *pc, int fd, uint32_t option,
                                uint32_t events) {

  DCHECK_NOTNULL(pc);

  epoll_event ev = {0};
  ev.data.ptr = pc;
  ev.events = events;

  epoll_ctl(epoll_fd_, option, fd, &ev);
}

void Dispatcher::ChannelHandelEvent(int n) {
  for (int i = 0; i < n; i++) {
    CHECK(epoll_lists_[i].data.ptr);

    Channels *chann = reinterpret_cast<Channels *>(epoll_lists_[i].data.ptr);
    CHECK(chann);
    // LOG(INFO) << epoll_lists_[i].events;
    chann->SetEvents(epoll_lists_[i].events);
    chann->HandleEvents();
  }
}

void Dispatcher::HandlelAsyncQueue() {
  exec_queue_mutex_.lock();
  if (task_wait_queue_.size())
    std::swap(task_exec_queue_, task_wait_queue_);
  exec_queue_mutex_.unlock();

  // LOG(INFO) << " async task queue size " << task_exec_queue_.size();
  for (auto &task : task_exec_queue_) {
    task();
  }
  task_exec_queue_.clear();
}
