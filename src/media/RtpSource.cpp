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

RtpSource::RtpSource() : send_cb_(nullptr) {}

RtpSource::~RtpSource() {}

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
    : rtp_packet_buf_(nullptr), time_stamp_(90000 * 0.005), seq_(0) {}

RtpSourceH264::~RtpSourceH264() {}

std::string RtpSourceH264::GetMediaDescription(uint16_t port) {
  char buf[100] = {0};
  sprintf(buf, "m=video %hu RTP/AVP 96", port); // \r\nb=AS:2000
  return std::string(buf);
}

std::string RtpSourceH264::GetAttribute() {
  return std::string("a=rtpmap:96 H264/90000");
}

void RtpSourceH264::SendFrameByRtpTcp(int channel_id, char *frame,
                                      int frame_len, bool keyframe) {
  // auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(
  //     std::chrono::steady_clock::now());
  // time_stamp_ =
  //     (uint32_t)((time_point.time_since_epoch().count() + 500) / 1000 * 90);

  if (!rtp_packet_buf_) {
    rtp_packet_buf_ = (char *)malloc(RTP_PACKET_MALLOC_SIZE);
    rtpHeaderInit((RtpPacketTcp *)rtp_packet_buf_, 0, 0, 0, RTP_VESION,
                  RTP_PAYLOAD_TYPE_H264, 0, 0, 0, 0x88923423);
  } else {
    memset(rtp_packet_buf_ + RTP_HEADER_SIZE + 4, 0, RTP_MAX_PKT_SIZE);
  }
  // LOG(INFO)<<"send h264 rtp packet;";
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
    rtp_packet_tcp->header[1] = 2 * channel_id;
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

  // LOG(INFO) << time_stamp_;
}

uint32_t RtpSourceAAC::AACSampleRate[16] = {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,  7350,  0,     0,     0 /*reserved */
};

RtpSourceAAC::RtpSourceAAC(int samplerate, int channels)
    : rtp_packet_buf_(nullptr), samplerate_(samplerate), channels_(channels),
      time_stamp_(500), seq_(0) {}

RtpSourceAAC::~RtpSourceAAC() {}

std::string RtpSourceAAC::GetMediaDescription(uint16_t port) {
  char buf[100] = {0};
  sprintf(buf, "m=audio %hu RTP/AVP 97", port); // \r\nb=AS:64
  return std::string(buf);
}

std::string RtpSourceAAC::GetAttribute() {
  char buf[500] = {0};
  sprintf(buf, "a=rtpmap:97 MPEG4-GENERIC/%u/%u\r\n", samplerate_, channels_);

  uint8_t index = 0;
  for (index = 0; index < 16; index++) {
    if (AACSampleRate[index] == samplerate_) {
      break;
    }
  }

  if (index == 16) {
    return ""; // error
  }

  uint8_t profile = 1;
  char config[10] = {0};

  sprintf(config, "%02x%02x", (uint8_t)((profile + 1) << 3) | (index >> 1),
          (uint8_t)((index << 7) | (channels_ << 3)));
  sprintf(buf + strlen(buf),
          "a=fmtp:97 profile-level-id=1;"
          "mode=AAC-hbr;"
          "sizelength=13;indexlength=3;indexdeltalength=3;"
          "config=%04u",
          atoi(config));

  return std::string(buf);
}

void RtpSourceAAC::SendFrameByRtpTcp(int channel_id, char *frame, int frame_len,
                                     bool key_frame) {
  // auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(
  //     std::chrono::steady_clock::now());
  // time_stamp_ = (uint32_t)((time_point.time_since_epoch().count() + 500) /
  //                          1000 * 48000 / 1000);

  if (!rtp_packet_buf_) {
    rtp_packet_buf_ = (char *)malloc(5 * 1024);
    // rtpHeaderInit((RtpPacketTcp *)rtp_packet_buf_, 0, 0, 0, RTP_VESION,
    // RTP_PAYLOAD_TYPE_AAC, 0, 0, 0, 0x88923423);
    rtpHeaderInit((RtpPacketTcp *)rtp_packet_buf_, 0, 0, 0, RTP_VESION,
                  RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);
  } else {
    memset(rtp_packet_buf_ + RTP_HEADER_SIZE + 4, 0, RTP_MAX_PKT_SIZE);
  }
  // LOG(INFO)<<"send aac rtp packet;";
  RtpPacketTcp *rtp_packet_tcp = (RtpPacketTcp *)rtp_packet_buf_;

  // char naluType = frame[0];

  // if (frame_len <= RTP_MAX_PKT_SIZE) {
  // nalu长度小于最大包场：单一NALU单元模式
  /*
   *   0 1 2 3 4 5 6 7 8 9
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |F|NRI|  Type   | a single NAL unit ... |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  rtp_packet_tcp->payload[0] = 0x00;
  rtp_packet_tcp->payload[1] = 0x10;
  rtp_packet_tcp->payload[2] = (frame_len & 0x1FE0) >> 5; //高8位
  rtp_packet_tcp->payload[3] = (frame_len & 0x1F) << 3;   //低5位
  // LOG(INFO) << frame_len;
  memcpy(rtp_packet_tcp->payload + 4, frame, frame_len);
  int packet_size = frame_len + RTP_HEADER_SIZE + 4;

  rtp_packet_tcp->header[0] = '$';
  rtp_packet_tcp->header[1] = 2 * channel_id;
  rtp_packet_tcp->header[2] = (packet_size & 0xFF00) >> 8;
  rtp_packet_tcp->header[3] = (packet_size)&0xFF;
  rtp_packet_tcp->rtpHeader.timestamp = htonl(time_stamp_);
  rtp_packet_tcp->rtpHeader.seq = htons(seq_++);

  if (send_cb_) {
    send_cb_(rtp_packet_buf_, packet_size + 4, key_frame);
  }
  time_stamp_ += 1025;

  seq_++;
  // }
}