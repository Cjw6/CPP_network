#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "Channels.h"
#include "Dispatcher.h"
#include "Buffer.h"

// #include "BufferWriter.h"


class TcpServer;

class TcpConnect : public Channels,
                   public std::enable_shared_from_this<TcpConnect> {
public:
  friend class TcpServer;

  using Ptr = std::shared_ptr<TcpConnect>;
  using ReadCb = std::function<int(TcpConnect::Ptr, BufferReader& , BufferWriter&)>;
  // using WriteCb = std::function<void(char *, int)>;
  using ErrorCb = std::function<void(TcpConnect::Ptr)>;

  TcpConnect(TcpServer *serv, Dispatcher::Ptr &disp, std::string ip, int port,
             int fd, std::string name);
  virtual ~TcpConnect();

  void HandleEvents() override;

  void SetReadCb(ReadCb cb) { read_cb_ = cb; }
  // void SetWriteCb(WriteCb cb) { write_cb_ = cb; }
  void SetErrorCb(ErrorCb cb) { error_cb_ = cb; }
  int Send(int n);

  const std::string &GetName() { return name_; }
  const std::string &GetIP() { return ip_; }
  int &GetPort() { return port_; }

private:
  int HandleRead();
  int HandleWrite();
  int HandleError();

  std::string name_;
  TcpServer *serv_;
  std::string ip_;
  int port_;

  BufferReader buffer_reader_;
  BufferWriter buffer_writer_;
  
  ReadCb read_cb_;
  // WriteCb write_cb_;
  ErrorCb error_cb_;

  
  std::atomic<bool> connected_;
};
