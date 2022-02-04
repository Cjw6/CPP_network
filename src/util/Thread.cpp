#include "Thread.h"
#include <sys/syscall.h>
#include <unistd.h>

static thread_local pid_t t_thread_id = 0;

pid_t GetThreadId() {
  if (__builtin_expect(t_thread_id == 0, 0)) {
    t_thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
  }
  return t_thread_id;
}

std::atomic<int> Thread::s_thread_cnt(2);

Thread::Thread() : running_(false), finished_(true) {}

Thread::~Thread() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

void Thread::Start(std::string name, bool is_detach) {
  finished_ = false;
  thread_ = std::thread([&] {
    PreRun();

    {
      std::lock_guard<std::mutex> lg(thread_mux_);
      running_ = true;
    }
    wait_thread_cond_.notify_one();

    Run();
    AfterRun();

    std::lock_guard<std::mutex> lg(thread_mux_);
    running_ = false;
    finished_ = true;
  });

  if (is_detach) {
    thread_.detach();
  }

  std::unique_lock<std::mutex> ul(thread_mux_);
  if (!running_ && !finished_) {
    wait_thread_cond_.wait(ul);
  }
}

void Thread::PreRun() { printf("prerun \n"); };

void Thread::Run() { printf("thread run %d \n", GetThreadId()); }

void Thread::AfterRun() { printf("afterrun\n"); }