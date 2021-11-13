#pragma once

#include "network/Channels.h"
#include "network/Dispatcher.h"
// #include "network/TcpServer.h"
#include <functional>
#include <memory>
#include <string>

class TcpServer;

class TcpConnect : public Channels,
                   public std::enable_shared_from_this<TcpConnect> {
public:
  using Ptr = std::shared_ptr<TcpConnect>;
  using ReadCb = std::function<int(TcpConnect::Ptr, char *, int)>;
  // using WriteCb = std::function<void(char *, int)>;
  using ErrorCb = std::function<void(TcpConnect::Ptr)>;

  TcpConnect(TcpServer *serv, Dispatcher::Ptr &disp, std::string ip, int port,
             int fd, std::string name);
  virtual ~TcpConnect();

  int HandleRead() override;
  int HandleWrite() override;
  int HandleError() override;

  void SetReadCb(ReadCb cb) { read_cb_ = cb; }
  // void SetWriteCb(WriteCb cb) { write_cb_ = cb; }
  void SetErrorCb(ErrorCb cb) { error_cb_ = cb; }
  int Send(int n);

  std::string &GetName() { return name_; }
  std::string &GetIP() { return ip_; }
  int &GetPort() { return port_; }

private:
  std::string name_;
  TcpServer *serv_;
  std::string ip_;
  int port_;

  // TODO:未来优化缓冲区的构成
  char buffer[512]; // temp buffer ...
  int index_;
  ReadCb read_cb_;
  // WriteCb write_cb_;
  ErrorCb error_cb_;
};
