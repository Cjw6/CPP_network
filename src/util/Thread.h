//
// Created by cjw on 21-3-31.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

pid_t GetThreadId();

class Thread {
public:
  Thread();
  virtual ~Thread();
  void Start(std::string name = "", bool is_detach = false);

protected:
  virtual void PreRun();
  virtual void Run();
  virtual void AfterRun();

private:
  // void RunInternal(std::string &name);
  std::thread thread_;
  std::mutex thread_mux_;
  std::condition_variable wait_thread_cond_;
  bool running_;
  bool finished_;

  static std::atomic<int> s_thread_cnt;
};
