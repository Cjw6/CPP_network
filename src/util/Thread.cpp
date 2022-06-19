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
