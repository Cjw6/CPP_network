#include "media/RtspServer.h"

#include "util/Log.h"
#include <gflags/gflags.h>

#include <signal.h>

DEFINE_string(logdir, "./log", "日志文件夹路径");
DEFINE_int32(thread_num, 1, "调度器的线程使用数");
DEFINE_string(bind_ip, "0.0.0.0", "绑定ip");
DEFINE_int32(bind_port, 8888, "绑定ip");

void SigiIntHandler(int sig) {
  LOG(ERROR) << "handle signal  int" << sig;

  static std::once_flag once_flag;
  std::call_once(once_flag, [] {
    LOG(INFO) << "shut down log";
    ::google::ShutdownGoogleLogging();
  });

  exit(0);
}

int main(int argc, char **argv) {
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  InitGlog(argv[0], FLAGS_logdir.c_str());

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SigiIntHandler);

  Dispatcher::Config dispatcher_conf;
  dispatcher_conf.thread_num = FLAGS_thread_num;
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  disp->InitLoop(dispatcher_conf);

  RtspServer serv;
  ServerConfig conf;
  conf.ip_ = FLAGS_bind_ip;
  conf.port = FLAGS_bind_port;
  conf.max_listen = 1024;
  LOG(INFO) << "rtsp server bind ip " << conf.ip_ << "   port: " << conf.port;
  serv.InitService(conf, disp);

  disp->Dispatch();
  return 0;
}