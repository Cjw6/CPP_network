
#include "network/Dispatcher.h"
#include "network/Socket.h"
#include "network/TcpConnect.h"
#include "network/TcpServer.h"

#include "util/Log.h"
#include <gflags/gflags.h>

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

#include "util/StringSplit.h"

DEFINE_string(logdir, "./log", "日志文件夹路径");
DEFINE_int32(thread_num, 1, "调度器的线程使用数");
DEFINE_string(bind_ip, "0.0.0.0", "绑定ip");
DEFINE_int32(bind_port, 8888, "绑定ip");

struct ClientManager {
  ClientManager();
  int accep_cnt;
};

ClientManager::ClientManager() : accep_cnt(0) {}

ClientManager g_cli_mgr;

int MessageHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                   BufferWriter &writer) {
  // for (int i = 0; i < 1000000; i++)
  writer.Append(reader.ReadBegin(), reader.ReadableSize());
  reader.RetrieveAll();
  return 1;
}

int HugeSendMegaEchoHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                            BufferWriter &writer) {
  for (int i = 0; i < 1000000; i++)
    writer.Append(reader.ReadBegin(), reader.ReadableSize());
  reader.RetrieveAll();
  return 1;
}

int HugeRecvMessageHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                           BufferWriter &writer) {
  return 1;
}

int main(int argc, char **argv) {
  ::google::ParseCommandLineFlags(&argc, &argv, false);
  InitGlog(argv[0], FLAGS_logdir.c_str());

  signal(SIGPIPE, SIG_IGN);

  Dispatcher::Config dispatcher_conf;
  dispatcher_conf.thread_num = FLAGS_thread_num;
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  disp->InitLoop(dispatcher_conf);

  TcpServer serv;
  serv.SetNewConnCb([](int fd, std::string ip, int port) {
    LOG(INFO) << "new conn" << fd << " " << ip << " " << port;
    LOG(INFO) << "accept cnt:" << ++g_cli_mgr.accep_cnt;
  });
  serv.SetReadCb(MessageHandler);
  serv.SetErrorCb(
      [](TcpConnect::Ptr p) { LOG(INFO) << p->GetName() << " error"; });

  ServerConfig conf;
  conf.ip_ = FLAGS_bind_ip;
  conf.port = FLAGS_bind_port;
  conf.max_listen = 1024;
  LOG(INFO) << "server bind ip " << conf.ip_ << "   port: " << conf.port;
  serv.InitServer(conf, disp);

  disp->Dispatch();
  return 0;
}