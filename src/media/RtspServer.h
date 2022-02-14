#pragma once

#include "media/MediaSession.h"
#include "network/TcpServer.h"

// using MediaId = int;

class RtspServer : public TcpServer {
public:
  RtspServer();
  ~RtspServer();

  void InitService(ServerConfig &config, Dispatcher::Ptr &disp);

  MediaSessionId AddMediaSess(MediaSession::Ptr &media_sess);
  MediaSession::Ptr FindMediaSessionByMediaId(MediaSessionId id);
  MediaSession::Ptr FindMediaSessionByRtspName(std::string &name);

  void PushFrame(MediaSessionId id, int channel_id, char *frame, int size,
                 bool key_frame);

protected:
  int SetNewConn(int fd, std::string &ip, int port) override;

private:
  std::map<std::string, MediaSessionId> rtsp_name_id_map_;
  std::map<MediaSessionId, MediaSession::Ptr> rtsp_id_media_map_;
};