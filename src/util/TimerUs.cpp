#include "Timer.h"

#include <sys/time.h>

TimeUs::TimeUs() : timestamp_(0) {}

TimeUs::TimeUs(int64_t time) : timestamp_(time) {}

TimeUs::~TimeUs() {}

TimeUs TimeUs::Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return TimeUs(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

void TimeUs::AddTime(int us) { timestamp_ += us; }

TimeUs TimeUs::operator-(TimeUs &t) {
  return TimeUs(timestamp_ - t.timestamp_);
}

bool TimeUs::operator==(TimeUs &t) { return timestamp_ == t.timestamp_; }

bool TimeUs::operator>(TimeUs &t) { return timestamp_ > t.timestamp_; }

bool TimeUs::operator<(TimeUs &t) { return timestamp_ < t.timestamp_; }
