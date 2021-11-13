#include "TcpServer.h"
#include "Socket.h"
#include "TcpConnect.h"
#include "network/Acceptor.h"
#include "network/Channels.h"

#include <glog/logging.h>
#include <memory>
#include <resolv.h>
#include <string>
#include <unistd.h>

TcpServer::TcpServer()
    : conn_gen_id_(0), new_conn_cb_(nullptr), read_cb_(nullptr),
      error_cb_(nullptr) {}

TcpServer::~TcpServer() {}

void TcpServer::SetNewConnCb(AcceptNewConnectCallback cb) { new_conn_cb_ = cb; }
void TcpServer::SetReadCb(TcpConnect::ReadCb cb) { read_cb_ = cb; }
void TcpServer::SetErrorCb(TcpConnect::ErrorCb cb) { error_cb_ = cb; }

void TcpServer::InitServer(Config &conf, Dispatcher::Ptr &disp) {
  conf_ = conf;
  dispatcher_ = disp;
  int fd = CreateServerSocket(conf.ip_, conf_.port, 0, conf.max_listen);
  if (fd > 0) {
    acceptor_ = std::make_unique<Acceptor>(dispatcher_, conf.ip_, conf.port,
                                           conf.max_listen, fd);
    acceptor_->SetNewConnCb([&](int fd, std::string ip, int port) {
      if (new_conn_cb_)
        new_conn_cb_(fd, ip, port);

      std::string conn_name = "tcp_conn" + std::to_string(conn_gen_id_++);
      TcpConnect::Ptr tcp_conn = std::make_shared<TcpConnect>(
          this, dispatcher_, ip, port, fd, conn_name);

      tcp_conn->SetReadCb([&](TcpConnect::Ptr p,char *buffer, int len) -> int {
        LOG(INFO) << "tcp server read cb";
        DCHECK(read_cb_) << "read_cb_ is null";
        return read_cb_(p,buffer, len);
      });

      tcp_conn->SetErrorCb([&](TcpConnect::Ptr p) {
        LOG(INFO) << "tcp server error cb";
        DCHECK(error_cb_) << " error_cb_ is null";
        error_cb_(p);
      });

      conn_map_.emplace(conn_name, tcp_conn);
    });
  }
}

void TcpServer::RemoveSocketByName(std::string name) { conn_map_.erase(name); }
