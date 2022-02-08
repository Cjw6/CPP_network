#pragma once

#include "network/TcpConnect.h"
#include <memory>

class RtpSession : public std::enable_shared_from_this<RtpSession> {
public:
  RtpSession(TcpConnect::Ptr tcp_conn, int rtp_socket);
  ~RtpSession();

  void SendRtpPacket(char *frame, int len, bool key_frame);

  void EnableHasKeyFrame(bool res) { has_key_frame_ = res; }
  bool HasKeyFrame() { return has_key_frame_; }
  void EnablePlay(bool res);
  bool IsPlay();

private:
  std::weak_ptr<TcpConnect> tcp_conn_;
  int rtp_socket_;
  bool play_;
  bool has_key_frame_;
};