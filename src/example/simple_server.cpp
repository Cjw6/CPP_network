
#include "network/Dispatcher.h"
#include "network/Socket.h"
#include "network/TcpConnect.h"
#include "network/TcpServer.h"
#include "util/Log.h"

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

const std::string log_dir = "./log";

struct ClientManager {
  ClientManager();
  int accep_cnt;
};

ClientManager::ClientManager() : accep_cnt(0) {}

ClientManager g_cli_mgr;

int MessageHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                   BufferWriter &writer) {
  // char *pmsg = reader.beginRead();
  // int size = reader.readableBytes();
  // writer.Append(pmsg, size);
  // reader.retrieveAll();
  return 1;
}

int HugeSendMessageHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                           BufferWriter &writer) {
  // LOG(INFO)<< "recv: "<<reader.retrieveAllAsString();
  // // int size = INT_MAX-1;
  // std::string pmsg;
  // for (int i = 0; i < 1024 * 1024 ; i++) {
  //   pmsg += char('0' + i % 10);
  // }
  // for (int i = 0; i < 10; i++)
  //   writer.Append(pmsg.data(), pmsg.length());

  return 1;
}

int HugeRecvMessageHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                           BufferWriter &writer) {
  // int save_errno = 0;
  // int res = 0;

  // //这里需要将数据全部读完 。。。。。。。。。
  // do {
  //   res = reader.readFd(tcp_comm->GetFd(), &save_errno);
  // } while (res >= 0);

  // reader.retrieveAll();

  return 1;
}

int main(int argc,char** argv) {
  InitGlog(argv[0], log_dir.c_str());
  signal(SIGPIPE, SIG_IGN);

  Dispatcher::Config dispatcher_conf;
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  disp->InitLoop(dispatcher_conf);

  TcpServer serv;
  serv.SetNewConnCb([](int fd, std::string ip, int port) {
    LOG(INFO) << "new conn" << fd << " " << ip << " " << port;
    LOG(INFO) << "accept cnt:" << ++g_cli_mgr.accep_cnt;
  });
  // serv.SetReadCb(MessageHandler);
  // serv.SetReadCb(HugeSendMessageHandler);
  serv.SetReadCb(HugeRecvMessageHandler);
  serv.SetErrorCb(
      [](TcpConnect::Ptr p) { LOG(INFO) << p->GetName() << " error"; });
  
  TcpServer::Config conf;
  conf.ip_ = "0.0.0.0";
  conf.port = 8888;
  conf.max_listen = 1024;
  serv.InitServer(conf, disp);

  disp->Dispatch();
  return 0;
}