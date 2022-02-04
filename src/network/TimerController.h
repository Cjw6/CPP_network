#include "Channels.h"

#include "util/TimerUs.h"

#include <functional>
#include <set>

class TimerTask {
public:
  using Func = std::function<void()>;

  TimerTask();
  TimerTask(TimeUs &t, Func func);
  ~TimerTask();

  void RunTask();

  bool operator>(TimerTask &task);
  bool operator<(TimerTask &task);

private:
  Func func_;
  TimeUs target_;
  TimeUs interval_;
};

class TimerController {
public:
  TimerController();
  ~TimerController();

  void AddTask(TimerTask::Func func, TimeUs &t);
  void Schedule(TimeUs t = TimeUs::Now());

private:
  int timer_fd_;
  std::set<TimerTask, std::less<>> timertasks_;
};