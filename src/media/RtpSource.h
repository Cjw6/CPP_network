#pragma once

#include "RtpPacket.h"
#include "network/TcpConnect.h"

class RtpSourceH264 {
public:
  using SendRtpPktCb = std::function<void(char *, int, bool)>;
  RtpSourceH264();
  ~RtpSourceH264();

  void SetSendRtpPacketCb(const SendRtpPktCb &cb) { send_cb_ = cb; }
  void SendFrameByRtpTcp(char *frame, int len, bool key_frame);

  static bool IsKeyFrame(char *data, int size);

private:
  // std::weak_ptr<TcpConnect> tcp_conn_;
  SendRtpPktCb send_cb_;
  char *rtp_packet_buf_;
  uint32_t time_stamp_;
  int seq_;
};