#include "network/Dispatcher.h"

#include <chrono>

#include <glog/logging.h>

int g_i = 0;

class A {
public:
  A() { LOG(INFO) << "A"; }
  ~A() { LOG(INFO) << "~A"; }
  void Print() { LOG(INFO) << "print a"; }
};

thread_local A g_a;

int main(int argc, char **argv) {
  using namespace std::chrono_literals;
  LOG(INFO) << "start";
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  Dispatcher::Config conf;
  conf.thread_num = 8;
  disp->Init(conf);

  std::thread th([&] { g_a.Print(); });
  th.detach();

  disp->timer_mgr_.AddFuncAfterDuration(Cj::TM_WM_Local, Cj::DurationMs(100),
                                        []() {
                                          LOG(INFO) << "timer1";
                                          return 1;
                                        });

  disp->timer_mgr_.AddRepeatedFunc(Cj::TM_WM_Local, Cj::DurationMs(1000), 5,
                                   [&]() { LOG(INFO) << g_i++; });

  disp->timer_mgr_.AddRepeatedFunc(Cj::TM_WM_ThrPool, Cj::DurationMs(2000), -1,
                                   []() { LOG(INFO) << __func__; });

  disp->Dispatch();
  return 0;
}