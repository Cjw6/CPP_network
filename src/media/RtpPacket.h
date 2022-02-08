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
