#pragma once

#include "util/Thread.h"
#include "util/ThreadPool.h"
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <sched.h>
#include <sys/epoll.h>
#include <vector>

using TaskCb = std::function<void()>;

class Channels;

class Dispatcher {
public:
  enum WorkMode { kSTMode, kMTMode };
  struct Config{
    Config();
    int thread_num;
    int epoll_max_listen;
  };

  using Ptr = std::shared_ptr<Dispatcher>;

  // 默认使用单线程模式 
  Dispatcher();
  ~Dispatcher();

  bool InitLoop(Config &config);
  void Dispatch();

  //事件更改 
  void AddEvent(int fd, int state);
  void DeleteEvent(int fd, int state);
  void ModifyEvent(int fd, int state);
  void UpdateChannels(Channels *pc);

  //从其他的线程添加任务。。。
  void AddTaskInDispatcher(TaskCb task);
  void ExecTask(TaskCb task);

  const WorkMode &GetWorkMode() { return work_mode_; }

private:
  void ChannelHandelEvent(int n);

  WorkMode work_mode_;
  int epoll_fd_;
  std::vector<epoll_event> epoll_lists_;

  // 用于单线程模式下的回调任务 
  std::mutex exec_queue_mutex_;
  std::deque<TaskCb> task_exec_queue_;
  std::deque<TaskCb> task_wait_queue_;

  pid_t thread_id_;
  std::atomic<int> running_flag_;

  std::unique_ptr<ThreadPool> pool_;
  int thread_num_;
  int epoll_max_listen_num_;
};
