#include "TimerUs.h"

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

void TimeUs::AddTime(const TimeUs &t) { AddTimeUs(t.GetTimeUs()); }

void TimeUs::AddTimeMs(int ms) { timestamp_ += ms * 1000; }

void TimeUs::AddTimeUs(int us) { timestamp_ += us; }

bool TimeUs::Valid() { return timestamp_ != 0; }

TimeUs TimeUs::operator+(const TimeUs &t) {
  return TimeUs(timestamp_ + t.timestamp_);
}

TimeUs TimeUs::operator-(const TimeUs &t) {
  return TimeUs(timestamp_ - t.timestamp_);
}

bool TimeUs::operator==(const TimeUs &t) { return timestamp_ == t.timestamp_; }

bool operator>(const TimeUs &l, const TimeUs &r) {
  return l.GetTimeUs() < r.GetTimeUs();
}

bool operator<(const TimeUs &l, const TimeUs &r) {
  return l.GetTimeUs() < r.GetTimeUs();
}