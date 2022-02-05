#pragma once

#include <cstdint>
#include <ctime>

class TimeUs {
public:
  TimeUs();
  TimeUs(int64_t time);
  ~TimeUs();

  int64_t GetTimeUs() const { return timestamp_; }
  void AddTime(const TimeUs &t);
  void AddTimeMs(int ms);
  void AddTimeUs(int us);
  bool Valid();

  TimeUs operator+(const TimeUs &t);
  TimeUs operator-(const TimeUs &t);
  bool operator==(const TimeUs &t);

  static TimeUs Now();
  static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t timestamp_;
};

bool operator>(const TimeUs &l, const TimeUs &r);
bool operator<(const TimeUs &l, const TimeUs &r);