#pragma once

#include "Dispatcher.h"
#include "Channels.h"
#include <string>
#include <memory>

using AcceptNewConnectCallback = std::function<void(int, std::string, int)>;

class Dispatcher;

class Acceptor:public Channels{
  public:
  using UPtr=std::unique_ptr<Acceptor>;
  Acceptor(Dispatcher::Ptr& disp,std::string ip,int port,int listen_num,int fd);
  ~Acceptor();
  int  HandleRead() override;
  void SetNewConnCb(AcceptNewConnectCallback cb);
  private:
  std::string ip_;
  int port_;
  AcceptNewConnectCallback accept_cb_;
};