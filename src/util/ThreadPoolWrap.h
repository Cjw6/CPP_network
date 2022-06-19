#ifndef THREADPOOL_CPP_WARP_H
#define THREADPOOL_CPP_WARP_H

#include <cstdio>
#include <functional>
#include <iterator>
#include <stdlib.h>

#include "thrdpool.h"

namespace Cj {

class ThreadPoolTask {
public:
  ThreadPoolTask(std::function<void()> &&cb) : cb_(std::move(cb)) {}
  ~ThreadPoolTask() {}
  std::function<void()> cb_;
};

inline void ThreadTaskRun(void *p) {
  ThreadPoolTask *th_task = reinterpret_cast<ThreadPoolTask *>(p);
  th_task->cb_();
  delete th_task;
}

template <class FUNC, class... ARGS>
void AddTaskToThreadPool(thrdpool_t *pool, FUNC &&func, ARGS &&...args) {
  auto &&tmp_func =
      std::bind(std::forward<FUNC>(func), std::forward<ARGS>(args)...);
  thrdpool_task task; // = (thrdpool_task *)malloc(sizeof(thrdpool_task));
  task.routine = ThreadTaskRun;
  task.context = new ThreadPoolTask(tmp_func);
  thrdpool_schedule(&task, pool);
}

} // namespace Cj

#endif