#pragma once

// #include "TimerController.h"
#include "util/ThreadPoolWrap.h"
#include <sched.h>
#include <sys/epoll.h>

#include <cstdint>

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

// enum EventOption;
class Channels;
class TimerController;
class WakeChannel;

using TaskCb = std::function<void()>;
class Dispatcher : public std::enable_shared_from_this<Dispatcher> {
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

  void Dispatch();
  bool InitLoop(Config &config);

  //事件更改
  void AddEvent(int fd, int state);
  void DeleteEvent(int fd, int state);
  void ModifyEvent(int fd, int state);
  void UpdateChannels(Channels *pc, int fd, uint32_t option, uint32_t events);

  void RunTask(const TaskCb &task, bool use_threadpool = false);
  void Wake();

  const WorkMode &GetWorkMode() { return work_mode_; }

private:
  enum UpdateRegisterTaskState { TaskAdd = 0, TaskRemove, TaskUnkonow };

  void ChannelHandelEvent(int n);
  void HandlelAsyncQueue();

  WorkMode work_mode_;

  int epoll_fd_;
  int epoll_max_listen_num_;
  std::vector<epoll_event> epoll_lists_;

  //回调任务
  std::deque<TaskCb> task_exec_queue_;
  std::mutex exec_queue_mutex_;

  pid_t thread_id_;
  std::atomic<int> running_flag_;

  thrdpool_t *thread_pool_;
  int thread_num_;

  std::unique_ptr<WakeChannel> wake_;
};
