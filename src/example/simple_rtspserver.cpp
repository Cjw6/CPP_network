#include "media/ReadH264File.h"
#include "media/RtspServer.h"
#include "util/Log.h"
#include "util/TimeCost.h"
#include "util/TimerUs.h"

#include <gflags/gflags.h>

#include <signal.h>
#include <thread>

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

  RtspServer rtsp_serv;
  MediaSession::Ptr media_session = std::make_shared<MediaSession>("live");
  MediaSessionId media_id = rtsp_serv.AddMediaSess(media_session);

  ServerConfig conf;
  conf.ip_ = FLAGS_bind_ip;
  conf.port = FLAGS_bind_port;
  conf.max_listen = 1024;
  LOG(INFO) << "rtsp server bind ip " << conf.ip_ << "   port: " << conf.port;
  rtsp_serv.InitService(conf, disp);

  std::thread push_thread([&]() {
    ReadH264File r;
    std::string path = "/media/cjw/work1/media/output2.h264";
    // std::string path = "resource/test.h264";
    r.Open(path);
    ByteBuffer buffer;
    int index = 0;

    TimeCost tc;
    while (1) {
      if (r.ReadFrame(buffer, 500 * 1024) < 0) {
        break;
      }

      bool key_frame = RtpSourceH264::IsKeyFrame(buffer.Begin(), buffer.Size());
      // LOG(INFO) << "read frame from file" << index++ << " key frame?"
      //           << key_frame << " frame_size" << buffer.ReadableSize();
      rtsp_serv.PushFrame(media_id, buffer.ReadBegin(), buffer.ReadableSize(),
                          key_frame);

      EC_SleepUs(1000 * 1000 / 25 - static_cast<int>(tc.duration_us()));
      tc.restart();
    }
  });

  disp->Dispatch();
  push_thread.join();
  return 0;
}
