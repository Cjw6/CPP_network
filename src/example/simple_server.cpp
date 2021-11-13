#include "network/Dispatcher.h"
#include "network/Socket.h"
#include "network/TcpConnect.h"
#include "network/TcpServer.h"
#include <cstddef>
#include <cstdio>
#include <glog/logging.h>
#include <memory>
#include <sys/socket.h>

// void NewConnCb

void NewConnCb(int fd, std::string ip, int port) {}

int main() {
  TcpServer::Config conf;
  conf.ip_ = "0.0.0.0";
  conf.port = 8888;
  conf.max_listen = 1024;
  Dispatcher::Ptr disp = std::make_shared<Dispatcher>();
  disp->InitLoop(2000);

  TcpServer serv;

  serv.SetNewConnCb([](int fd, std::string ip, int port) {
    LOG(INFO) << "new conn" << fd << " " << ip << " " << port;
  });

  serv.SetReadCb([](TcpConnect::Ptr p, char *buf, int len) -> int {
    LOG(INFO) << "read " << len << " " << buf;
    return  p->Send(len);
  });

  serv.SetErrorCb(
      [](TcpConnect::Ptr p) { LOG(INFO) << p->GetName() << " error"; });

  serv.InitServer(conf, disp);
  disp->Dispatch();
  return 0;
}