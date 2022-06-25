#include "network/Dispatcher.h"
#include "util/Log.h"

#include <chrono>
#include <thread>

int main(int argc, char **argv) {
  LOG(INFO) << "start";
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  Dispatcher::Config conf;
  disp->Init(conf);

  std::thread t1([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    int cnt = 3;
    while (cnt > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      LOG(INFO) << "wake1";
      disp->Wake();
      cnt--;
    }

    cnt = 1000;
    while (cnt > 0) {
      LOG(INFO) << "wake2";
      disp->Wake();
      cnt--;
    }
  });

  disp->Dispatch();
  t1.join();
  return 0;
}