#include "TimerController.h"
#include "util/Log.h"

#include <algorithm>

std::atomic<int64_t> TimerTask::kTimerId(0);

TimerTask::TimerTask() : func_(nullptr), timer_id(-1) {}

TimerTask::TimerTask(Id id) : func_(nullptr), timer_id(id) {}

TimerTask::TimerTask(TimeUs &time)
    : target_(time), func_(nullptr), timer_id(-1) {}

TimerTask::TimerTask(Func func, TimeUs &delay, bool singleshot)
    : func_(std::move(func)) {
  DCHECK(func_);
  target_ = TimeUs::Now() + delay;
  if (!singleshot) {
    interval_ = delay;
  }
  timer_id = kTimerId.fetch_add(1);
}

TimerTask::~TimerTask() {}

// TimerTask::TimerTask(TimerTask &&task) {
//   func_ = std::move(task.func_);
//   target_ = task.target_;
//   interval_ = task.interval_;
// }

bool TimerTask::RunTask() {
  if (func_) {
    if (func_() < 0) {
      return false;
    }
  }

  if (interval_.Valid()) {
    auto new_target = target_ + interval_;
    auto now = TimeUs::Now();
    if (now > new_target) {
      target_ = now;
      target_.AddTime(interval_);
    } else {
      target_ = new_target;
    }
    return true;
  } else {
    return false;
  }
}

bool operator>(const TimerTask &l, const TimerTask &r) {
  return l.GetTargetTimeUs() > r.GetTargetTimeUs();
}

bool operator<(const TimerTask &l, const TimerTask &r) {
  return l.GetTargetTimeUs() < r.GetTargetTimeUs();
}

bool operator==(const TimerTask &l, const TimerTask &r) {
  return l.GetId() == r.GetId();
}

TimerController::TimerController() {}

TimerController::~TimerController() {}

TimerTask::Id TimerController::AddTask(TimerTask::Func func, TimeUs &delay,
                                       bool singleshot) {
  auto res = timertasks_.emplace(std::move(func), delay, singleshot);
  return res.first->GetId();
}

bool TimerController::RemoveTimerTask(TimerTask::Id &id) {
  // auto res= timertasks_.find(TimerTask(id));
  // std::find_first_of(_ExecutionPolicy &&__exec, _ForwardIterator1 __first, _ForwardIterator1 __last, _ForwardIterator2 __s_first, _ForwardIterator2 __s_last)
  auto res = std::find_if(
      timertasks_.begin(), timertasks_.end(),
      [id](const TimerTask &task) -> bool { return task.GetId() == id; });

  if (res == timertasks_.end()) {
    id = -1;
    return false;
  } else {
    id = -1;
    timertasks_.erase(res);
    return true;
  }
}

void TimerController::Schedule() {
  if (timertasks_.empty()) {
    return;
  }

  auto now = TimeUs::Now();

  TimerTask cmptask(now);

  std::vector<TimerTask> taskvct;
  auto begin = timertasks_.begin();
  auto end = timertasks_.lower_bound(cmptask);

  taskvct.insert(taskvct.end(), begin, end);
  timertasks_.erase(begin, end);

  for (auto &it : taskvct) {
    if (it.RunTask()) {
      timertasks_.emplace(it);
    }
  }
}

int TimerController::CalIntervalMs() {
  if (timertasks_.empty()) {
    return -1;
  }

  auto now = TimeUs::Now();
  auto begin = timertasks_.begin();

  if (begin->GetTargetTimeUs() < now) {
    return 0;
  } else {
    return (begin->GetTargetTimeUs() - now.GetTimeUs()) / 1000;
  }
}
