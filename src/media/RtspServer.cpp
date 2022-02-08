#include "RtspServer.h"
#include "RtspSession.h"

#include <glog/logging.h>

// int RtspServer::MediaSessIdGen = 0;

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

  conn_mgr_->AddConnectToManager(sess->GetName(), sess);
  return 1;
}

MediaSessionId RtspServer::AddMediaSess(MediaSession::Ptr &media_sess) {
  if (!media_sess) {
    return -1;
  }
  rtsp_name_id_map_.emplace(media_sess->GetSessName(), media_sess->GetSessId());
  rtsp_id_media_map_.emplace(media_sess->GetSessId(), media_sess);
  return media_sess->GetSessId();
}

MediaSession::Ptr RtspServer::FindMediaSessionByMediaId(MediaSessionId id) {
  auto it = rtsp_id_media_map_.find(id);
  if (it == rtsp_id_media_map_.end()) {
    return MediaSession::Ptr();
  } else {
    return it->second;
  }
}

MediaSession::Ptr RtspServer::FindMediaSessionByRtspName(std::string &name) {
  auto it = rtsp_name_id_map_.find(name);
  if (it == rtsp_name_id_map_.end()) {
    return MediaSession::Ptr();
  } else {
    return FindMediaSessionByMediaId(it->second);
  }
}

void RtspServer::PushFrame(MediaSessionId id, char *frame, int size,
                           bool keyframe) {
  auto it = rtsp_id_media_map_.find(id);
  if (it == rtsp_id_media_map_.end()) {
    return;
  }

  auto &media_sess = it->second;
  if (!media_sess) {
    return;
  }

  media_sess->PushFrame(frame, size, keyframe);
}
