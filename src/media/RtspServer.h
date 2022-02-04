#pragma once

#include "network/TcpServer.h"

class RtspServer : public TcpServer {
public:
  RtspServer();
  ~RtspServer();

  void InitService(ServerConfig &config, Dispatcher::Ptr &disp);

protected:
  int SetNewConn(int fd, std::string &ip, int port) override;

private:
  // int ReadHandler(TcpConnect::Ptr tcp_comm, BufferReader &reader,
                  // BufferWriter &writer);
};