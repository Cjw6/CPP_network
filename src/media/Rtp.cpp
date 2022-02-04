#include "Rtp.h"

#include <glog/logging.h>

ReadH264File::ReadH264File() : fp_(nullptr) {}

ReadH264File::~ReadH264File() {
  if (fp_) {
    fclose(fp_);
  }
}

int ReadH264File::Open(std::string &path) {
  fp_ = fopen(path.c_str(), "r");
  if (!fp_) {
    LOG(ERROR) << "open file:" << path << " fail";
    return -1;
  }
  return 1;
}

int ReadH264File::startCode3(char *buf) {
  if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
    return 1;
  else
    return 0;
}

int ReadH264File::startCode4(char *buf) {
  if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
    return 1;
  else
    return 0;
}

int ReadH264File::ReadFrame(ByteBuffer &piece) {
  if (!fp_) {
    return -1;
  }

  piece.Resize(500 * 1024);

  int n = fread(piece.Begin(), 1, piece.Capcity(), fp_);
  if (n <= 0) {
    fclose(fp_);
    fp_ = nullptr;
    return -1;
  }

  LOG(INFO) << "read n:" << n;

  char *data = piece.Begin();
  int start_code = 0;
  if (startCode3(data)) {
    start_code = 3;
  } else if (startCode4(data)) {
    start_code = 4;
  } else {
    return -1;
  }

  piece.RetrieveAll();
  piece.SetReadIndex(start_code);

  data += start_code;
  char *header = data;

  while (header - piece.Begin() < n - 3) {
    if (startCode3(header)) {
      piece.SetWriteIndex(header - piece.Begin());
      fseek(fp_, header - piece.Begin() - n, SEEK_CUR);
      LOG(INFO) << header - data;
      return 1;
    } else if (startCode4(header)) {
      piece.SetWriteIndex(header - piece.Begin());
      fseek(fp_, header - piece.Begin() - n, SEEK_CUR);
      LOG(INFO) << header - data;
      return 1;
    } else {
      header++;
    }
  }

  return -1;
}

RtpSenderBase::RtpSenderBase() {}

RtpSenderBase::~RtpSenderBase() {}

RtChanneleBase::RtChanneleBase(RtpSenderBase *sender) : sender_(sender) {
  DCHECK(sender_);
}

RtChanneleBase::~RtChanneleBase() {}

int RtChanneleBase::Init(RtpConfig &conf) {
  rtp_config_ = conf;
  sender_->rtpHeaderInit(conf.packet_malloc_size, 0, conf.extension,
                         conf.padding, conf.version, conf.payloadType,
                         conf.marker, 0, 0, 0);
  return 1;
}

RtpChannelH264::RtpChannelH264(RtpSenderBase *sender)
    : RtChanneleBase(sender) {}

RtpChannelH264::~RtpChannelH264() {}

int RtpChannelH264::SendRtpPacket(char *frame, int frame_size,
                                  uint32_t timestamp) {
  return 1;
}

RtpSenderTcp::RtpSenderTcp() {}

RtpSenderTcp::~RtpSenderTcp() {}

void RtpSenderTcp::rtpHeaderInit(int packet_size, uint8_t csrcLen,
                                 uint8_t extension, uint8_t padding,
                                 uint8_t version, uint8_t payloadType,
                                 uint8_t marker, uint16_t seq,
                                 uint32_t timestamp, uint32_t ssrc) {
  rtp_packet_ = (RtpPacketTcp *)malloc(packet_size);
  if (!rtp_packet_) {
    LOG(FATAL) << "rtp packet malloc fail";
  }
  memset(rtp_packet_, 9, packet_size);

  rtp_packet_->rtpHeader.csrcLen = csrcLen;
  rtp_packet_->rtpHeader.extension = extension;
  rtp_packet_->rtpHeader.padding = padding;
  rtp_packet_->rtpHeader.version = version;
  rtp_packet_->rtpHeader.payloadType = payloadType;
  rtp_packet_->rtpHeader.marker = marker;
  rtp_packet_->rtpHeader.ssrc = ssrc;
  rtp_packet_->rtpHeader.timestamp = timestamp;
  rtp_packet_->rtpHeader.ssrc = ssrc;
}

int RtpSenderTcp::SendRtpPacket(char *frame, int frame_size,
                                uint32_t timestamp) {
  return 1;
}
