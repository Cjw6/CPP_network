#include "media/RtpSource.h"

#include "util/Log.h"
#include <arpa/inet.h>

#define RTP_VESION 2
#define RTP_PAYLOAD_TYPE_H264 96
#define RTP_PAYLOAD_TYPE_AAC 97

static const int RTP_MAX_PKT_SIZE = 1400;
static const int RTP_HEADER_SIZE = 12;
static const int RTP_PACKET_MALLOC_SIZE =
    RTP_MAX_PKT_SIZE + RTP_HEADER_SIZE + 20;

static void rtpHeaderInit(struct RtpPacketTcp *rtpPacket, uint8_t csrcLen,
                          uint8_t extension, uint8_t padding, uint8_t version,
                          uint8_t payloadType, uint8_t marker, uint16_t seq,
                          uint32_t timestamp, uint32_t ssrc) {
  rtpPacket->rtpHeader.csrcLen = csrcLen;
  rtpPacket->rtpHeader.extension = extension;
  rtpPacket->rtpHeader.padding = padding;
  rtpPacket->rtpHeader.version = version;
  rtpPacket->rtpHeader.payloadType = payloadType;
  rtpPacket->rtpHeader.marker = marker;
  rtpPacket->rtpHeader.seq = seq;
  rtpPacket->rtpHeader.timestamp = timestamp;
  rtpPacket->rtpHeader.ssrc = ssrc;
}

bool RtpSourceH264::IsKeyFrame(char *data, int size) {
  if (size > 4) {
    // 0x67:sps ,0x65:IDR, 0x6: SEI
    // printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x
    // \n",(int)data[0],(int)data[1],(int)data[2],(int)data[3],(int)data[4],(int)data[5]);
    if (data[4] == 0x67 || data[4] == 0x65 || data[4] == 0x6 ||
        data[4] == 0x27) {
      return true;
    }
  }
  return false;
}

RtpSourceH264::RtpSourceH264()
    : send_cb_(nullptr), rtp_packet_buf_(nullptr), time_stamp_(0), seq_(0) {}

RtpSourceH264::~RtpSourceH264() {}

void RtpSourceH264::SendFrameByRtpTcp(char *frame, int frame_len,
                                      bool keyframe) {
  if (!rtp_packet_buf_) {
    rtp_packet_buf_ = (char *)malloc(RTP_PACKET_MALLOC_SIZE);
    rtpHeaderInit((RtpPacketTcp *)rtp_packet_buf_, 0, 0, 0, RTP_VESION,
                  RTP_PAYLOAD_TYPE_H264, 0, 0, 0, 0x88923423);
  } else {
    memset(rtp_packet_buf_ + RTP_HEADER_SIZE + 4, 0, RTP_MAX_PKT_SIZE);
  }

  RtpPacketTcp *rtp_packet_tcp = (RtpPacketTcp *)rtp_packet_buf_;

  char naluType = frame[0];

  if (frame_len <= RTP_MAX_PKT_SIZE) {
    // nalu长度小于最大包场：单一NALU单元模式
    /*
     *   0 1 2 3 4 5 6 7 8 9
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  |F|NRI|  Type   | a single NAL unit ... |
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */
    memcpy(rtp_packet_tcp->payload, frame, frame_len);
    int packet_size = frame_len + RTP_HEADER_SIZE;
    rtp_packet_tcp->header[0] = '$';
    rtp_packet_tcp->header[1] = 0;
    rtp_packet_tcp->header[2] = (packet_size & 0xFF00) >> 8;
    rtp_packet_tcp->header[3] = (packet_size)&0xFF;
    rtp_packet_tcp->rtpHeader.timestamp = htonl(time_stamp_);
    rtp_packet_tcp->rtpHeader.seq = htons(seq_++);

    if (send_cb_) {
      send_cb_(rtp_packet_buf_, packet_size + 4, keyframe);
    }

  } else {
    //这里是处理分包的情况
    int pktNum = frame_len / RTP_MAX_PKT_SIZE;        // 有几个完整的包
    int remainPktSize = frame_len % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
    int i, pos = 1;

    for (i = 0; i < pktNum; i++) {
      rtp_packet_tcp->payload[0] = (naluType & 0x60) | 28;
      rtp_packet_tcp->payload[1] = naluType & 0x1F;

      if (i == 0)                                     //第一包数据
        rtp_packet_tcp->payload[1] |= 0x80;           // start
      else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
        rtp_packet_tcp->payload[1] |= 0x40;           // end

      memcpy(rtp_packet_tcp->payload + 2, frame + pos, RTP_MAX_PKT_SIZE);

      int packet_size = RTP_MAX_PKT_SIZE + RTP_HEADER_SIZE + 2;

      rtp_packet_tcp->header[0] = '$';
      rtp_packet_tcp->header[1] = 0;
      rtp_packet_tcp->header[2] = (packet_size & 0xFF00) >> 8;
      rtp_packet_tcp->header[3] = (packet_size)&0xFF;
      rtp_packet_tcp->rtpHeader.timestamp = htonl(time_stamp_);
      rtp_packet_tcp->rtpHeader.seq = htons(seq_++);

      seq_++;
      pos += RTP_MAX_PKT_SIZE;
      if (send_cb_) {
        send_cb_(rtp_packet_buf_, packet_size + 4, keyframe);
      }
    }

    if (remainPktSize > 0) {
      rtp_packet_tcp->payload[0] = (naluType & 0x60) | 28;
      rtp_packet_tcp->payload[1] = naluType & 0x1F;
      rtp_packet_tcp->payload[1] |= 0x40; // end

      memcpy(rtp_packet_tcp->payload + 2, frame + pos, remainPktSize + 2);
      int packet_size = remainPktSize + 2 + RTP_HEADER_SIZE + 2;

      rtp_packet_tcp->header[0] = '$';
      rtp_packet_tcp->header[1] = 0;
      rtp_packet_tcp->header[2] = (packet_size & 0xFF00) >> 8;
      rtp_packet_tcp->header[3] = (packet_size)&0xFF;
      rtp_packet_tcp->rtpHeader.timestamp = htonl(time_stamp_);
      rtp_packet_tcp->rtpHeader.seq = htons(seq_++);

      if (send_cb_) {
        send_cb_(rtp_packet_buf_, packet_size + 4, keyframe);
      }
      seq_++;
    }
  }

  time_stamp_ += 90000 / 25;
}
// {

//   memcpy(rtp_packet_tcp->payload, frame, frame_size);

//   int packet_size = frame_size + RTP_HEADER_SIZE;
//   rtp_packet_tcp->header[0] = '$';
//   rtp_packet_tcp->header[1] = 0;
//   rtp_packet_tcp->header[2] = (packet_size & 0xFF00) >> 8;
//   rtp_packet_tcp->header[3] = (packet_size)&0xFF;

//   int send_size = packet_size + 4;

//   if(socket_session_map_.size()){
//     auto it=socket_session_map_.begin();
//     it.
//   }

//   rtp_packet_tcp->rtpHeader.seq = htons(rtp_packet_tcp->rtpHeader.seq);
//   rtp_packet_tcp->rtpHeader.timestamp =
//       htonl(rtp_packet_tcp->rtpHeader.timestamp);
//   rtp_packet_tcp->rtpHeader.ssrc = htonl(rtp_packet_tcp->rtpHeader.ssrc);

// }
// }
