#pragma once

#include "RtpPacket.h"
#include "network/TcpConnect.h"

class RtpSource {
public:
  using SendRtpPktCb = std::function<void(char *, int, bool)>;

  RtpSource();
  virtual ~RtpSource();

  void SetSendRtpPacketCb(const SendRtpPktCb &cb) { send_cb_ = cb; }
  virtual void SendFrameByRtpTcp(char *frame, int len, bool key_frame) = 0;
  virtual std::string GetMediaDescription(uint16_t port) = 0;
  virtual std::string GetAttribute() = 0;

protected:
  SendRtpPktCb send_cb_;
};

class RtpSourceH264 : public RtpSource {
public:
  RtpSourceH264();
  ~RtpSourceH264();

  void SendFrameByRtpTcp(char *frame, int len, bool key_frame) override;
  std::string GetMediaDescription(uint16_t port) override;
  std::string GetAttribute() override;

  static bool IsKeyFrame(char *data, int size);

private:
  char *rtp_packet_buf_;
  uint32_t time_stamp_;
  int seq_;
};

class RtpSourceAAC : public RtpSource {
public:
  RtpSourceAAC(int samplerate, int channels);
  ~RtpSourceAAC();

  void SendFrameByRtpTcp(char *frame, int len, bool key_frame) override;
  std::string GetMediaDescription(uint16_t port) override;
  std::string GetAttribute() override;

  static uint32_t AACSampleRate[16];

private:
  char *rtp_packet_buf_;
  uint32_t samplerate_ = 44100;
  uint32_t channels_ = 2;
  uint32_t time_stamp_;
  int seq_;
};
