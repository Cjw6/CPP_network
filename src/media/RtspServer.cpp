#include "RtspServer.h"
#include "RtspSession.h"

#include <glog/logging.h>

RtspServer::RtspServer() {}

RtspServer::~RtspServer() {}

void RtspServer::InitService(ServerConfig &config, Dispatcher::Ptr &disp) {
  InitServer(config, disp);
  SetReadCb([this](TcpConnect::Ptr tcp_comm, BufferReader &reader,
                   BufferWriter &writer) -> int {
    LOG(INFO) << "handle new conn";
    RtspSession::Ptr rtsp_sess =
        std::dynamic_pointer_cast<RtspSession>(tcp_comm);
    DCHECK(rtsp_sess) << " rtsp sess is null";
    return rtsp_sess->ReadHandler();
  });
}

int RtspServer::SetNewConn(int fd, std::string &ip, int port) {
  LOG(INFO) << "new rtsp connect " << fd << " " << ip << " " << port;
  std::string conn_name = "tcp_conn" + std::to_string(conn_gen_id_++);
  auto sess =
      std::make_shared<RtspSession>(this, dispatcher_, ip, port, fd, conn_name);

  sess->SetReadCb([&](TcpConnect::Ptr p, BufferReader &reader,
                      BufferWriter &writer) -> int {
    LOG(INFO) << "tcp server read cb";
    if (read_cb_)
      return read_cb_(p, reader, writer);
    else
      return -1;
  });

  sess->SetErrorCb([&](TcpConnect::Ptr p) {
    LOG(INFO) << "tcp server error cb";
    // DCHECK(error_cb_) << " error_cb_ is null";
    if (error_cb_)
      error_cb_(p);
  });

  conn_mgr_->AddConnectToManager(sess->GetName(), sess);

  return 1;
}
