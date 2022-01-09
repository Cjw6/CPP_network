//
// Created by cjw on 21-7-5.
//

#ifndef CWORK_TIMECOST_H
#define CWORK_TIMECOST_H

#include <chrono>
#include <cstdint>
#include <glog/logging.h>
#include <iostream>
#include <ratio>

// 这里使用 steady clock 是比较准确的

class TimeCostTool {
public:
  TimeCostTool(std::string name = "")
      : name_(name), p(std::chrono::steady_clock::now()) {}
  ~TimeCostTool() = default;
  void set_name(std::string str) { name_ = std::move(str); }
  void restart() { p = std::chrono::steady_clock::now(); }

  //返回微秒
  float duration_us(bool log = false) {
    auto end = std::chrono::steady_clock::now(); //结束时间
    float cost_time = std::chrono::duration<float, std::micro>(end - p).count();
    if (log)
      LOG(INFO) << name_ << " cost time:" << cost_time / 1000.0 << " ms";
    return cost_time;
  }

  //返回毫秒
  float duration_ms(bool log = false) {
    auto end = std::chrono::steady_clock::now(); //结束时间
    float cost_time = std::chrono::duration<float, std::milli>(end - p).count();
    if (log)
      LOG(INFO) << name_ << " cost time:" << cost_time << " ms";
    return cost_time;
  }

private:
  std::string name_;
  std::chrono::time_point<std::chrono::steady_clock> p;
};

#endif // CWORK_TIMECOST_H
