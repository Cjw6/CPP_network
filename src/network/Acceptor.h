#pragma once

#include "Channels.h"
#include "Dispatcher.h"
#include <memory>
#include <string>

using AcceptNewConnectCallback = std::function<void(int, std::string, int)>;

class Dispatcher;

class Acceptor : public Channels {
public:
  using UPtr = std::unique_ptr<Acceptor>;
  // static std::string class_name;
  Acceptor(Dispatcher::Ptr &disp, std::string ip, int port, int listen_num);
  virtual ~Acceptor();
  int Listen();
  int GetFd() override { return listem_fd_; }
  // std::string & GetType() override;
  // int  HandleRead() override;
  void HandleEvents() override;
  void SetNewConnCb(AcceptNewConnectCallback cb);

private:
  int HandleRead();
  void HandleError();

  std::string ip_;
  int port_;
  int listen_num_;
  int listem_fd_;
  AcceptNewConnectCallback accept_cb_;
};