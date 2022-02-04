#pragma once

#include <cstdint>
#include <ctime>

class TimeUs {
public:
  TimeUs();
  TimeUs(int64_t time);
  ~TimeUs();

  int64_t GetAllUs() { return timestamp_; }
  void AddTime(int us);

  TimeUs operator-(TimeUs &t);
  bool operator==(TimeUs &t);
  bool operator>(TimeUs &t);
  bool operator<(TimeUs &t);

  static TimeUs Now();
  static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t timestamp_;
};
