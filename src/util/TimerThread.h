#ifndef __TIMER__
#define __TIMER__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#include <glog/logging.h>

#include "SafeMap.h"
#include "ThreadPoolWrap.h"

namespace Cj {

enum TimerWorkMode { TM_WM_Local, TM_WM_ThrPool };
using TimerTask = std::function<void>;

// https://en.cppreference.com/w/cpp/chrono/duration
using DurationMs = std::chrono::duration<int, std::milli>;
using DurationUs = std::chrono::duration<int, std::micro>;

class TimerQueueThread {
public:
  struct InternalS {
    std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
    std::function<void()> func_;
    TimerWorkMode work_mode_;
    int repeated_id;
    bool operator<(const InternalS &b) const {
      return time_point_ > b.time_point_;
    }
  };

public:
  void SetPool(thrdpool_t *pool) { pool_ = pool; }

  bool Run() {
    std::thread([this]() { RunLocal(); }).detach();
    return true;
  }

  bool IsPoolAvailable() { return !!pool_; }

  int Size() { return queue_.size(); }

  void Stop() {
    running_.store(false);
    cond_.notify_all();
    // thread_pool_.ShutDown();
  }

  template <typename R, typename P, typename F, typename... Args>
  void AddFuncAfterDuration(TimerWorkMode mode,
                            const std::chrono::duration<R, P> &time, F &&f,
                            Args &&...args) {
    InternalS s;
    s.work_mode_ = mode;
    s.time_point_ = std::chrono::high_resolution_clock::now() + time;
    s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
    cond_.notify_all();
  }

  template <typename F, typename... Args>
  void AddFuncAtTimePoint(
      TimerWorkMode mode,
      const std::chrono::time_point<std::chrono::high_resolution_clock>
          &time_point,
      F &&f, Args &&...args) {
    InternalS s;
    s.work_mode_ = mode;
    s.time_point_ = time_point;
    s.func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
    cond_.notify_all();
  }

  template <typename R, typename P, typename F, typename... Args>
  int AddRepeatedFunc(TimerWorkMode mode,
                      const std::chrono::duration<R, P> &time, int repeat_num,
                      F &&f, Args &&...args) {
    int id = GetNextRepeatedFuncId();
    repeated_id_state_map_.Emplace(id, RepeatedIdState::kRunning);
    auto tem_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    if (repeat_num > 0) {
      AddRepeatedCntFuncLocal(mode, repeat_num - 1, time, id,
                              std::move(tem_func));
    } else {
      AddRepeatedFuncLocal(mode, time, id, std::move(tem_func));
    }

    return id;
  }

  void CancelRepeatedFuncId(int func_id) {
    repeated_id_state_map_.EraseKey(func_id);
  }

  int GetNextRepeatedFuncId() { return repeated_func_id_++; }

  TimerQueueThread(thrdpool_t *pool = nullptr) : pool_(pool) {
    repeated_func_id_.store(0);
    running_.store(true);
  }

  ~TimerQueueThread() { Stop(); }

  enum class RepeatedIdState { kInit = 0, kRunning = 1, kStop = 2 };

private:
  void RunLocal() {
    while (running_.load()) {
      std::unique_lock<std::mutex> lock(mutex_);
      if (queue_.empty()) {
        cond_.wait(lock);
        continue;
      }
      auto s = queue_.top();
      auto diff = s.time_point_ - std::chrono::high_resolution_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >
          0) {
        cond_.wait_for(lock, diff);
        continue;
      } else {
        queue_.pop();
        lock.unlock();
        if (s.work_mode_ == TM_WM_Local || !pool_) {
          s.func_();
        } else {
          Cj::AddTaskToThreadPool(pool_, std::move(s.func_));
        }
      }
    }
  }

  template <typename R, typename P, typename F>
  void AddRepeatedCntFuncLocal(TimerWorkMode mode, int repeat_num,
                               const std::chrono::duration<R, P> &time, int id,
                               F &&f) {
    if (!this->repeated_id_state_map_.IsKeyExist(id)) {
      return;
    }
    InternalS s;
    s.time_point_ = std::chrono::high_resolution_clock::now() + time;
    auto tem_func = std::move(f);
    s.repeated_id = id;
    s.func_ = [this, &tem_func, repeat_num, time, id,
               workmode = s.work_mode_]() {
      tem_func();
      if (!this->repeated_id_state_map_.IsKeyExist(id)) {
        return;
      } else if (repeat_num == 0) {
        CancelRepeatedFuncId(id);
        return;
      }
      AddRepeatedCntFuncLocal(workmode, repeat_num - 1, time, id,
                              std::move(tem_func));
    };
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
    lock.unlock();
    cond_.notify_all();
  }

  template <typename R, typename P, typename F>
  void AddRepeatedFuncLocal(TimerWorkMode mode,
                            const std::chrono::duration<R, P> &time, int id,
                            F &&f) {
    if (!this->repeated_id_state_map_.IsKeyExist(id)) {
      return;
    }
    InternalS s;
    s.time_point_ = std::chrono::high_resolution_clock::now() + time;
    auto tem_func = std::move(f);
    s.repeated_id = id;
    s.func_ = [this, &tem_func, time, id, s, workmode = s.work_mode_]() {
      tem_func();
      if (!this->repeated_id_state_map_.IsKeyExist(id)) {
        return;
      }
      AddRepeatedFuncLocal(workmode, time, id, std::move(tem_func));
    };
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(s);
    lock.unlock();
    cond_.notify_all();
  }

private:
  std::priority_queue<InternalS> queue_;
  std::atomic<bool> running_;
  std::mutex mutex_;
  std::condition_variable cond_;
  thrdpool_t *pool_;
  std::atomic<int> repeated_func_id_;
  wzq::ThreadSafeMap<int, RepeatedIdState> repeated_id_state_map_;
};

} // namespace Cj

#endif