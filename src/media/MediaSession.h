#pragma once

#include "media/RtpSource.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

class RtpSession;
using RtpSessionPtr = std::shared_ptr<RtpSession>;

using MediaSessionId = int;

class MediaSession {
public:
  using Ptr = std::shared_ptr<MediaSession>;
  using NotifyConnCb = std::function<void()>;
  using NotifyDisconCb = std::function<void()>;

  MediaSession(std::string sess_name);
  ~MediaSession();

  void AddRtpSource(RtpSourceH264 *source);
  void SetNotifConnCb(NotifyConnCb &cb) { conn_cb_ = cb; }
  void SetNotifyDisconCb(NotifyDisconCb &cb) { discon_cb_ = cb; }

  const std::string &GetSessName() { return sess_name_; }
  const int &GetSessId() { return sess_id_; }

  int UserNum();

  void AddRtpSession(int socket_fd, RtpSessionPtr &rtp);
  void RemoveRtpSession(int socket_fd);
  void PushFrame(char *frame, int size,bool key_frame);

  static std::atomic<MediaSessionId> IdGen;

private:
  std::string sess_name_;
  MediaSessionId sess_id_;

  NotifyConnCb conn_cb_;
  NotifyDisconCb discon_cb_;

  RtpSourceH264 *source_;
  
  std::mutex session_map_mutex_;
  std::map<int, RtpSessionPtr> socket_session_map_;
  
  char *rtp_packet_buf_;
};