#pragma once

#include "Dispatcher.h"
#include "TcpConnect.h"
#include "network/Acceptor.h"
#include "network/Channels.h"
#include "network/ConnectManager.h"
#include <map>
#include <mutex>
#include <string>

class Acceptor;

struct ServerConfig {
  std::string ip_;
  int port;
  int max_listen;
  int max_epoll_num;
  int tcp_keep_alive_ms;
};

class TcpServer {
public:
  TcpServer();
  ~TcpServer();

  int InitServer(ServerConfig &conf, Dispatcher::Ptr &disp);
  void RemoveSocketByName(const std::string &name);

  void SetNewConnCb(AcceptNewConnectCallback cb);
  void SetReadCb(TcpConnect::ReadCb cb);
  void SetErrorCb(TcpConnect::ErrorCb cb);

protected:
  void SetConfigParams(ServerConfig &conf);
  virtual int SetNewConn(int fd, std::string &ip, int port);

  ServerConfig conf_;
  Acceptor::UPtr acceptor_;
  Dispatcher::Ptr dispatcher_;

  AcceptNewConnectCallback new_conn_cb_;
  TcpConnect::ReadCb read_cb_;
  TcpConnect::ErrorCb error_cb_;

  std::unique_ptr<TcpConnectManager> conn_mgr_;

  uint32_t register_tcpcheck_id_;

  int conn_gen_id_;
};