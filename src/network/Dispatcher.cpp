#include "network/Dispatcher.h"
#include "network/Channels.h"
#include "network/WakeUp.h"

#include "util/Thread.h"
#include "util/ThreadPoolWrap.h"

#include "util/Log.h"

#include <functional>
#include <memory>
#include <mutex>
#include <sys/epoll.h>

constexpr int kDefaultThreadStackSize = 4 * 1024;

static constexpr unsigned int str2int(const char *str, int h = 0) {
  return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

Dispatcher::Config::Config() : thread_num(1), epoll_max_listen(1000) {}

Dispatcher::Dispatcher()
    : epoll_fd_(-1), thread_id_(-1), running_flag_(0), thread_num_(0),
      epoll_max_listen_num_(0), thread_pool_(nullptr) {}

Dispatcher::~Dispatcher() {}

bool Dispatcher::Init(Config &conf) {
  thread_num_ = conf.thread_num;
  epoll_max_listen_num_ = conf.epoll_max_listen;

  CHECK(thread_num_ > 0);
  CHECK(epoll_max_listen_num_ > 10);

  if (thread_num_ == 1) {
    work_mode_ = kSTMode;
    LOG(INFO) << "cur work mode: ST";
  } else {
    work_mode_ = kMTMode;
    thread_pool_ = thrdpool_create(thread_num_, kDefaultThreadStackSize);
    timer_mgr_.SetPool(thread_pool_);
    LOG(INFO) << "cur work mode MT";
  }

  epoll_lists_.resize(epoll_max_listen_num_);
  epoll_fd_ = epoll_create(epoll_max_listen_num_);
  if (epoll_fd_ < 0) {
    LOG(ERROR) << "create epoll fail ...";
    return false;
  }

  auto self = shared_from_this();
  wake_ = std::make_unique<WakeChannel>(self);
  if (wake_->InitEventFd() < 0) {
    return false;
  }
  return true;
}

void Dispatcher::Dispatch() {
  thread_id_ = GetThreadId();
  running_flag_ = 1;
  // int interval = timer_ctl_->CalIntervalMs();
  // LOG(INFO) << "fisrt interval " << interval;
  int event_count = 0;
  int64_t loop_cnt = 0;

  HandlelAsyncQueue();
  timer_mgr_.Run();
  while (running_flag_) {
    event_count =
        epoll_wait(epoll_fd_, &epoll_lists_[0], epoll_lists_.size(), -1);

    if (event_count < 0) {
      PLOG(ERROR) << "epoll wait:";
      if (errno == EINTR) {

      } else {
        LOG(ERROR) << "dispatcher quit\n";
        return;
      }
    }
    ChannelHandelEvent(event_count);
    HandlelAsyncQueue();
    loop_cnt++;
  }
  LOG(INFO) << "quit";
}

void Dispatcher::RunTask(const TaskCb &task, bool use_threadpool) {
  if (work_mode_ == kMTMode && use_threadpool) {
    // pool_->enqueue(std::move(task));
    Cj::AddTaskToThreadPool(thread_pool_, task);
  } else if (thread_id_ == GetThreadId()) {
    task();
  } else {
    {
      std::lock_guard<std::mutex> lg(exec_queue_mutex_);
      task_exec_queue_.emplace_back(std::move(task));
    }
    wake_->WakeUp();
  }
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
  decltype(task_exec_queue_) task_queue_tmp;

  exec_queue_mutex_.lock();
  if (task_exec_queue_.size()) {
    task_queue_tmp.swap(task_exec_queue_);
  }
  exec_queue_mutex_.unlock();

  for (auto &task : task_queue_tmp) {
    task();
  }
}
