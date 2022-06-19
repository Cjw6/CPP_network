#include "util/ThreadPoolWrap.h"
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

#include "unistd.h"

std::atomic_int g_i;
void PrintI() { printf("print i:%d\n", g_i++); }

void PrintI2(int i) { printf("print i:%d\n", i); }

void PendingAllTasK(const thrdpool_task *ptask) {
  printf("use pending\n");
  ptask->routine(ptask->context);
}

// std::thread  g_th;
std::vector<thrdpool_task> g_tasks;

void PendingAllTasK2(const thrdpool_task *ptask) {
  // 这里模拟将任务 放到其他的线程执行  。。。。
  g_tasks.push_back(*ptask);
}

int main() {
  thrdpool_t *pool = thrdpool_create(8, 1024);
  for (int i = 0; i < 10; i++)
    Cj::AddTaskToThreadPool(pool, PrintI);
  for (int i = 0; i < 20; i++) {
    Cj::AddTaskToThreadPool(pool, PrintI2, i);
  }
  thrdpool_destroy(PendingAllTasK2, pool);

  auto &&func = [tasks(std::move(g_tasks))] {
    printf("#################run lambda########################\n");
    for (const auto &t : tasks) {
      t.routine(t.context);
    }
  };
  std::thread t1(func);
  if (t1.joinable()) {
    t1.join();
  }
  return 0;
}

/* g++   -ggdb -fsanitize=leak -fno-omit-frame-pointer -static-libstdc++
 * -static-libgcc -static-liblsan -lrt main.cpp thrdpool.c -lpthread */