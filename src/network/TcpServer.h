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

class TcpServer {
public:
  struct Config {
    std::string ip_;
    int port;
    int max_listen;
    int max_epoll_num;
  };
  TcpServer();
  ~TcpServer();

  void InitServer(Config &conf, Dispatcher::Ptr &disp);
  void RemoveSocketByName(const std::string &name);

  void SetNewConnCb(AcceptNewConnectCallback cb);
  void SetReadCb(TcpConnect::ReadCb cb);
  void SetErrorCb(TcpConnect::ErrorCb cb);

private:
  Config conf_;
  Acceptor::UPtr acceptor_;
  Dispatcher::Ptr dispatcher_;
  
  AcceptNewConnectCallback new_conn_cb_;
  TcpConnect::ReadCb  read_cb_;
  TcpConnect::ErrorCb  error_cb_;

  ConnectManager conn_mgr_;
  
  int conn_gen_id_;

  
};