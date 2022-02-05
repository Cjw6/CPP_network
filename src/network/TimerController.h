#include "Channels.h"

#include "util/TimerUs.h"

#include <atomic>
#include <functional>
#include <set>

class TimerTask {
public:
  using Id = int64_t;
  using Func = std::function<int()>;

  TimerTask();
  TimerTask(Id id);
  TimerTask(TimeUs &time);
  TimerTask(Func func, TimeUs &delay, bool singleshot);
  TimerTask(const TimerTask &task) = default;
  ~TimerTask();

  bool operator==(const TimerTask &r) { return GetId() == r.GetId(); }

  bool RunTask();
  TimeUs GetTargetTime() const { return target_; }
  int64_t GetTargetTimeUs() const { return target_.GetTimeUs(); }
  Id GetId() const { return timer_id; }

  static std::atomic<int64_t> kTimerId;

private:
  Func func_;
  TimeUs target_;
  TimeUs interval_;
  Id timer_id;
};

bool operator>(const TimerTask &l, const TimerTask &r);
bool operator<(const TimerTask &l, const TimerTask &r);
bool operator==(const TimerTask &l, const TimerTask &r);

class TimerController {
public:
  TimerController();
  ~TimerController();

  TimerTask::Id AddTask(TimerTask::Func func, TimeUs &delay, bool singleshot);
  bool RemoveTimerTask(TimerTask::Id &id);
  void Schedule();
  int CalIntervalMs();

private:
  int timer_fd_;
  std::set<TimerTask, std::less<TimerTask>> timertasks_;
};