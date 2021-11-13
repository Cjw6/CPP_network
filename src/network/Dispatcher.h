#pragma once

// #include "Channels.h"
#include "util/Thread.h"

#include <functional>
#include <memory>
#include <sched.h>
#include <sys/epoll.h>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>

using TaskCb=std::function<void()>;

class Channels;

class Dispatcher{
  public:
  using Ptr = std::shared_ptr<Dispatcher>;
  Dispatcher();
  ~Dispatcher();

  bool InitLoop(int epoll_num);
  void Dispatch();

  // fd event option
  void AddEvent(int fd, int state);
  void DeleteEvent(int fd, int state);
  void ModifyEvent(int fd, int state);
  void UpdateChannels(Channels *pc);

  void AddTask(TaskCb task);

  int epoll_fd_;
  std::vector<epoll_event> epoll_lists_;

  std::mutex exec_queue_mutex_;
  std::deque<TaskCb> task_exec_queue_;
  std::deque<TaskCb> task_wait_queue_;

  pid_t thread_id_;
  std::atomic<int> running_flag_;
};

