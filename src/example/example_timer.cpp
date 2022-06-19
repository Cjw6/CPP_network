#include "network/Dispatcher.h"

#include <chrono>

#include <glog/logging.h>

int main(int argc, char **argv) {
  #if 0
  LOG(INFO) << "start";
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  Dispatcher::Config conf;
  disp->InitLoop(conf);

  disp->AddTimerTask(
      []() {
        LOG(INFO) << "timer1";
        return 1;
      },
      1 * 1000 * 1000);

  int cnt = 3;
  TimerTask::Id timer_id = disp->AddTimerTask(
      [&]() {
        LOG(INFO) << "timer2  " << cnt;
        if (cnt == 0) {
          return -1;
        }
        cnt--;
        return 1;
      },
      2 * 1000 * 1000, false);

  int cnt2 = 0;
  TimerTask::Id timer_id2 = disp->AddTimerTask(
      [&]() {
        LOG(INFO) << "timer3  " << cnt2;
        cnt2++;
        return 1;
      },
      2 * 1000 * 1000, false);

  std::thread t1([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    disp->RunTask([&] { disp->RemoveTimerTask(timer_id2); });
  });

  disp->Dispatch();
  t1.join();
  #endif
  return 0;
}