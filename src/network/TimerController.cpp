#include "TimerController.h"

TimerTask::TimerTask() : func_(nullptr) {}

TimerTask::TimerTask(TimeUs &t, Func func)
    : target_(t), func_(std::move(func)) {}

TimerTask::~TimerTask() {}

void TimerTask::RunTask() {
  if (func_) {
    func_();
  }
}

TimerController::TimerController() {}

TimerController::~TimerController() {}

void TimerController::AddTask(TimerTask::Func func, TimeUs &t) {
  timertasks_.emplace(func, t);
}

void TimerController::Schedule(TimeUs t) {
  auto sets = timertasks_.lower_bound(t);
  // if()
}