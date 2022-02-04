#pragma once

#include <cstdint>
#include <string>

#include "util/ByteBuffer.h"

/*
 *
 *    0                   1                   2                   3
 *    7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           timestamp                           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |           synchronization source (SSRC) identifier            |
 *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *   |            contributing source (CSRC) identifiers             |
 *   :                             ....                              :
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
struct RtpHeader {
  /* byte 0 */
  uint8_t csrcLen : 4;
  uint8_t extension : 1;
  uint8_t padding : 1;
  uint8_t version : 2;

  /* byte 1 */
  uint8_t payloadType : 7;
  uint8_t marker : 1;

  /* bytes 2,3 */
  uint16_t seq;

  /* bytes 4-7 */
  uint32_t timestamp;

  /* bytes 8-11 */
  uint32_t ssrc;
};

enum RtpType { RtpTypeUdp = 0, RtpTypeTcp };

struct RtpPacketTcp {
  char header[4];
  struct RtpHeader rtpHeader;
  uint8_t payload[0];
};

struct RtpPacketUdp {
  struct RtpHeader rtpHeader;
  uint8_t payload[0];
};

struct RtpPacket {
  // RtpType type;
  union {
    RtpPacketTcp rtp_packet_tcp;
    RtpPacketUdp rtp_packet_udp;
  };
};

struct RtpTcpConfig {
  int socket;
  uint8_t rtp_channel;
};

struct RtpUdpConfig {
  int socket;
  char ip[40];
  int16_t port;
};

struct RtpConfig {
  RtpType type;
  union {
    RtpTcpConfig rtp_tcp_conf;
    RtpUdpConfig rtp_udp_conf;
  };
  // rtsp header
  uint8_t csrcLen;
  uint8_t extension;
  uint8_t padding;
  uint8_t version;
  uint8_t payloadType;
  uint8_t marker;

  // packet size
  int packet_malloc_size;
};

class ReadH264File {
public:
  ReadH264File();
  ~ReadH264File();
  int Open(std::string &path);
  int ReadFrame(ByteBuffer &piece);

private:
  static int startCode3(char *buf);
  static int startCode4(char *buf);

  FILE *fp_;
  // ByteBuffer buf_;
};

class RtpSenderBase {
public:
  RtpSenderBase();
  virtual ~RtpSenderBase();

  virtual int SendRtpPacket(char *frame, int frame_size,
                            uint32_t timestamp) = 0;
  virtual void rtpHeaderInit(int payload_size, uint8_t csrcLen,
                             uint8_t extension, uint8_t padding,
                             uint8_t version, uint8_t payloadType,
                             uint8_t marker, uint16_t seq, uint32_t timestamp,
                             uint32_t ssrc) = 0;
};

class RtChanneleBase {
public:
  RtChanneleBase(RtpSenderBase *sender);
  virtual ~RtChanneleBase();

  int Init(RtpConfig &conf);
  virtual int SendRtpPacket(char *frame, int frame_size,
                            uint32_t timestamp) = 0;

protected:
  RtpConfig rtp_config_;
  RtpSenderBase *sender_;
};

class RtpChannelH264 : public RtChanneleBase {
public:
  RtpChannelH264(RtpSenderBase *sender);
  ~RtpChannelH264();

  int SendRtpPacket(char *frame, int frame_size, uint32_t timestamp) override;

private:
};

class RtpSenderTcp : public RtpSenderBase {
public:
  RtpSenderTcp();
  ~RtpSenderTcp();
  void rtpHeaderInit(int payload_size, uint8_t csrcLen, uint8_t extension,
                     uint8_t padding, uint8_t version, uint8_t payloadType,
                     uint8_t marker, uint16_t seq, uint32_t timestamp,
                     uint32_t ssrc) override;
  
  int SendRtpPacket(char *data, int frame_size, uint32_t timestamp) override;

private:
  RtpPacketTcp *rtp_packet_;
  int rtp_packet_size_;
};
