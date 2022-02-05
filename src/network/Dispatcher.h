#pragma once

#include "TimerController.h"
#include "util/Thread.h"
#include "util/ThreadPool.h"

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <sched.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <vector>

// enum EventOption;
class Channels;
class TimerController;

using TaskCb = std::function<void()>;
class Dispatcher {
public:
  enum WorkMode { kSTMode, kMTMode };
  struct Config {
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
  void UpdateChannels(Channels *pc, int fd, uint32_t option, uint32_t events);

  //从其他的线程添加任务。。。
  void RunTask(TaskCb task, bool use_threadpool = false);
  TimerTask::Id AddTimerTask(TimerTask::Func func, TimeUs delay, bool singleshot = true);
  void RemoveTimerTask(TimerTask::Id &id);

  const WorkMode &GetWorkMode() { return work_mode_; }

private:
  void ChannelHandelEvent(int n);

  WorkMode work_mode_;

  int epoll_fd_;
  int epoll_max_listen_num_;
  std::vector<epoll_event> epoll_lists_;

  //回调任务
  std::mutex exec_queue_mutex_;
  std::deque<TaskCb> task_exec_queue_;
  std::deque<TaskCb> task_wait_queue_;

  pid_t thread_id_;
  std::atomic<int> running_flag_;

  std::unique_ptr<ThreadPool> pool_;
  int thread_num_;

  std::unique_ptr<TimerController> timer_ctl_;
};
