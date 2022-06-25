#include "media/ReadAACFile.h"
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
  std::string test_url = "live";
  LOG(INFO) << "test address:  rtsp://" << FLAGS_bind_ip << ":"
            << FLAGS_bind_port << "/" << test_url;

  InitGlog(argv[0], FLAGS_logdir.c_str());

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SigiIntHandler);

  Dispatcher::Config dispatcher_conf;
  dispatcher_conf.thread_num = FLAGS_thread_num;
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  disp->Init(dispatcher_conf);

  RtspServer rtsp_serv;
  MediaSession::Ptr media_session = std::make_shared<MediaSession>(test_url);
  media_session->AddRtpSource(MEDIA_SESS_CHANNEL_0, new RtpSourceH264);
  media_session->AddRtpSource(MEDIA_SESS_CHANNEL_1, new RtpSourceAAC(48000, 2));
  MediaSessionId media_id = rtsp_serv.AddMediaSess(media_session);

  ServerConfig conf;
  conf.ip_ = FLAGS_bind_ip;
  conf.port = FLAGS_bind_port;
  conf.max_listen = 1024;
  conf.tcp_keep_alive_ms = -1;
  LOG(INFO) << "rtsp server bind ip " << conf.ip_ << "   port: " << conf.port;
  rtsp_serv.InitService(conf, disp);

  std::thread push_video_thread([&]() {
    // return;

    ReadH264File r;
    std::string path = "/media/cjw/work1/media/output2.h264";
    // std::string path = "resource/test2.h264";
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
      rtsp_serv.PushFrame(media_id, MEDIA_SESS_CHANNEL_0, buffer.ReadBegin(),
                          buffer.ReadableSize(), key_frame);
      int interval_us =
          1000 * 1000 / 25 - static_cast<int>(tc.duration_us()) - 2000;
      // LOG(INFO) << "capture interval:" << interval_us;
      Cj::SleepUs(interval_us);
      // EC_SleepUs(interval_us);
      tc.restart();
    }
  });

  /*
   * 如果采样频率是44100
   * 一般AAC每个1024个采样为一帧
   * 所以一秒就有 44100 / 1024 = 43帧
   * 时间增量就是 44100 / 43 = 1025
   * 一帧的时间为 1 / 43 = 23ms
   */

  std::thread push_audio_thread([&]() {
    ReadAACFile r;
    std::string path = "resource/test.aac";
    // std::string path = "resource/test.h264";
    r.Open(path);
    ByteBuffer buffer;
    int index = 0;

    TimeCost tc;
    while (1) {
      if (r.ReadFrame(buffer, 5 * 1024) < 0) {
        break;
      }

      // bool key_frame = RtpSourceH264::IsKeyFrame(buffer.Begin(),
      // buffer.Size()); LOG(INFO) << "read frame from file" << index++ << " key
      // frame?"
      //           << key_frame << " frame_size" << buffer.ReadableSize();
      rtsp_serv.PushFrame(media_id, MEDIA_SESS_CHANNEL_1, buffer.Begin() + 7,
                          buffer.ReadableSize() - 7, false);
      int interval_us =
          1000 * 1000 / (48000 / 1024) - static_cast<int>(tc.duration_us());
      // LOG(INFO) << "capture interval:" << interval_us;
      // EC_SleepUs(interval_us);
      Cj::SleepUs(interval_us);
      tc.restart();
    }
  });

  disp->Dispatch();
  push_video_thread.join();
  push_audio_thread.join();
  return 0;
}

// 播放地址 ffplay   -rtsp_transport tcp  rtsp://127.0.0.1:8888/live  测试通过
// FIXME：vlc   rtsp://127.0.0.1:8888/live  --rtsp-tcp  -vv  测试未通过 

