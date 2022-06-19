#include "TcpServer.h"
#include "Acceptor.h"
#include "Buffer.h"
#include "Channels.h"
#include "Socket.h"
#include "TcpConnect.h"

#include <glog/logging.h>
#include <memory>
#include <mutex>
#include <resolv.h>
#include <string>
#include <unistd.h>

TcpServer::TcpServer()
    : conn_gen_id_(0), new_conn_cb_(nullptr), read_cb_(nullptr),
      error_cb_(nullptr){}

TcpServer::~TcpServer() {}

void TcpServer::SetNewConnCb(AcceptNewConnectCallback cb) {
  new_conn_cb_ = std::move(cb);
}

void TcpServer::SetReadCb(TcpConnect::ReadCb cb) { read_cb_ = std::move(cb); }

void TcpServer::SetErrorCb(TcpConnect::ErrorCb cb) {
  error_cb_ = std::move(cb);
}

int TcpServer::InitServer(ServerConfig &conf, Dispatcher::Ptr &disp) {
  SetConfigParams(conf);
  dispatcher_ = disp;

  acceptor_ = std::make_unique<Acceptor>(dispatcher_, conf_.ip_, conf_.port,
                                         conf_.max_listen);
  if (acceptor_->Listen() < 0) {
    return -1;
  }
  acceptor_->SetNewConnCb(
      [&](int fd, std::string ip, int port) { SetNewConn(fd, ip, port); });

  conn_mgr_ = std::make_unique<TcpConnectManager>();
  return 1;
}

void TcpServer::RemoveSocketByName(const std::string &name) {
  conn_mgr_->RemoveConnectToManager(name);
}

void TcpServer::SetConfigParams(ServerConfig &conf) {
  if (conf.ip_.empty()) {
    conf.ip_ = "0.0.0.0";
  }
  if (conf.port <= 0) {
    conf.port = 12300;
  }
  if (conf.max_listen == 0) {
    conf.max_listen = 1000;
  }
  if (conf.max_epoll_num) {
    conf.max_epoll_num = 1024;
  }
  if (conf.tcp_keep_alive_ms == 0) {
    conf.tcp_keep_alive_ms = 1000 * 30;
  }
  conf_ = conf;
}

int TcpServer::SetNewConn(int fd, std::string &ip, int port) {
  if (new_conn_cb_)
    new_conn_cb_(fd, ip, port);

  std::string conn_name = "tcp_conn" + std::to_string(conn_gen_id_++);
  TcpConnect::Ptr tcp_conn =
      std::make_shared<TcpConnect>(this, dispatcher_, ip, port, fd, conn_name);

  tcp_conn->SetReadCb([&](TcpConnect::Ptr p, BufferReader &reader,
                          BufferWriter &writer) -> int {
    LOG(INFO) << "tcp server read cb";
    DCHECK(read_cb_) << "read_cb_ is null";
    return read_cb_(p, reader, writer);
  });

  tcp_conn->SetErrorCb([&](TcpConnect::Ptr p) {
    LOG(INFO) << "tcp server error cb";
    DCHECK(error_cb_) << " error_cb_ is null";
    error_cb_(p);
  });

  conn_mgr_->AddConnectToManager(tcp_conn->GetName(), tcp_conn);

  return 0;
}
