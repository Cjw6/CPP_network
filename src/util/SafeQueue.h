//
// Created by cjw on 21-7-3.
//

#ifndef CWORK_SAFEQUEUE_H
#define CWORK_SAFEQUEUE_H

#include <condition_variable>
#include <deque>
#include <glog/logging.h>
#include <iostream>
#include <mutex>

template <typename T> class BlockQueue {
public:
  BlockQueue(/* args */) {}

  ~BlockQueue() {}

  int Len() {
    std::lock_guard<std::mutex> lg(mutex_);
    return deque_.size();
  }

  int RelaxedLen() { return deque_.size(); }

  bool FastPeek(T &value) {
    std::lock_guard<std::mutex> lg(mutex_);
    if (deque_.size()) {
      value = deque_[0];
      deque_.pop_front();
      return true;
    }
    return false;
  }

  bool Peek(T &value) {
    std::unique_lock<std::mutex> ul(mutex_);
    while (deque_.empty() && !quit_flag_) {
      empty_cond_.wait(ul);
    }
    if (quit_flag_)
      return false;
    value = deque_[0];
    deque_.pop_front();
    return true;
  }

  bool MovePeek(T &value) {
    std::unique_lock<std::mutex> ul(mutex_);
    while (deque_.empty() && !quit_flag_) {
      empty_cond_.wait(ul);
    }

    if (quit_flag_)
      return false;

    value = std::move(deque_[0]);
    deque_.pop_front();
    return true;
  }

  void Push(const T &node, bool block) {
    std::unique_lock<std::mutex> ul(mutex_);
    deque_.push_back(node);
    if (block)
      empty_cond_.notify_one();
  }

  //即使是在shutdown的情况下 不超出 太多的size 依然将原先阻塞的 push进行下去
  void Push(T &&node, bool block) {
    std::unique_lock<std::mutex> ul(mutex_);
    deque_.push_back(node);
    if (block)
      empty_cond_.notify_one();
  }

  void Shutdown() {
    std::lock_guard<std::mutex> lg(mutex_);
    quit_flag_ = true;
    empty_cond_.notify_all();
  }

private:
  bool quit_flag_ = false;
  std::deque<T> deque_;
  std::mutex mutex_;
  std::condition_variable empty_cond_;
};

#endif // CWORK_SAFEQUEUE_H
