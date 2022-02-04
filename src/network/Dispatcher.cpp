#include "Dispatcher.h"

#include "Channels.h"
#include "util/Thread.h"
#include "util/ThreadPool.h"
#include <functional>
#include <glog/logging.h>
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
  return true;
}

void Dispatcher::Dispatch() {
  thread_id_ = GetThreadId();

  running_flag_ = 1;
  while (running_flag_) {
    for (auto task : task_exec_queue_) {
      task();
    }

    for (auto task : task_wait_queue_) {
      task();
    }

    int event_count = 0;
    do {
      event_count =
          epoll_wait(epoll_fd_, &epoll_lists_[0], epoll_lists_.size(), -1);
    } while (event_count < 0 && errno == EINTR);

    if (event_count < 0) {
      PLOG(ERROR) << "epoll wait:";
    }

    ChannelHandelEvent(event_count);
  }
}

void Dispatcher::AddTaskInDispatcher(TaskCb task) {
  if (thread_id_ == GetThreadId()) {
    task();
  } else {
    std::lock_guard<std::mutex> lg(exec_queue_mutex_);
    task_wait_queue_.emplace_back(task);
  }
}

void Dispatcher::ExecTask(TaskCb task) {
  if (work_mode_ == kSTMode) {
    task();
  } else {
    pool_->enqueue(std::move(task));
  }
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

    chann->SetEvents(epoll_lists_[i].events);
    chann->HandleEvents();
  }
}

// void Dispatcher::EventMultiThread(int n) {
//   LOG(INFO) << "EventMultiThread events n:" << n;
//   for (int i = 0; i < n; i++) {
//     CHECK(epoll_lists_[i].data.ptr);

//     Channels *chann = reinterpret_cast<Channels *>(epoll_lists_[i].data.ptr);
//     LOG(INFO) << chann->GetType();

//     chann->SetEvents(epoll_lists_[i].events);
//     chann->HandleEvents();
//   }
// }

// void Dispatcher::SpecialDispatch(Channels *p) {
//   switch (str2int(p->GetType().c_str())) {
//   case (str2int("Acceptor")):
//     p->HandleEvents();
//     break;
//   default:
//     LOG(INFO)<<"post task...";
//     pool_->AddTask(std::bind(&Channels::HandleEvents, p));
//     break;
//   }
// }