#include "media/MediaSession.h"
#include "media/RtpSession.h"
#include "media/RtpSource.h"

#include "util/Log.h"
#include "util/SafeMem.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>

std::atomic<MediaSessionId> MediaSession::IdGen(0);

MediaSession::MediaSession(std::string name)
    : sess_name_(std::string(name)), sess_id_(IdGen.fetch_add(1)),
      conn_cb_(nullptr), discon_cb_(nullptr), source_(new RtpSourceH264),
      rtp_packet_buf_(nullptr) {
  source_->SetSendRtpPacketCb([this](char *data, int len, bool key_frame) {
    std::lock_guard<std::mutex> lg(session_map_mutex_);
    // LOG(INFO) << "send rtp packet ...";
    for (auto &[k, v] : socket_session_map_) {
      v->SendRtpPacket(data, len, key_frame);
    }
  });
}

MediaSession::~MediaSession() { SafeDelete(source_); }

int MediaSession::UserNum() {
  std::lock_guard<std::mutex> lg(session_map_mutex_);
  return socket_session_map_.size();
}

void MediaSession::AddRtpSource(RtpSourceH264 *source) {}

void MediaSession::AddRtpSession(int socket_fd, RtpSessionPtr &rtp) {
  if (conn_cb_) {
    conn_cb_();
  }
  LOG(INFO) << "add rtp_session into  media_session";
  std::lock_guard<std::mutex> lg(session_map_mutex_);
  socket_session_map_.emplace(socket_fd, rtp);
}

void MediaSession::RemoveRtpSession(int socket_fd) {
  std::lock_guard<std::mutex> lg(session_map_mutex_);
  auto iter = socket_session_map_.find(socket_fd);
  if (iter == socket_session_map_.end()) {
    return;
  }
  socket_session_map_.erase(iter);
}

void MediaSession::PushFrame(char *frame, int frame_size, bool key_frame) {
  if (UserNum()) {
    // LOG(INFO) << "push frame to send ....";
    if (source_) {
      source_->SendFrame(frame, frame_size, key_frame);
    }
  }
}

// if (!rtp_packet_buf_) {
//   rtp_packet_buf_ = (char *)malloc(RTP_MAX_PKT_SIZE);
// } else {
//   memset(rtp_packet_buf_, 0, RTP_MAX_PKT_SIZE);
// }

// RtpPacketTcp *rtp_packet_tcp = (RtpPacketTcp *)rtp_packet_buf_;
// rtpHeaderInit(rtp_packet_tcp, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264,
// 0,
//               0, 0, 0x88923423);

// // RtpPacketTcp *rtp_pkt =
// //     (RtpPacketTcp *)malloc(sizeof(fg)) uint8_t naluType; //
// nalu第一个字节 int sendBytes = 0;
// // int ret;
// char naluType = frame[0];
// if (frame_size <= RTP_MAX_PKT_SIZE) //
// nalu长度小于最大包场：单一NALU单元模式
// {
//   /*
//    *   0 1 2 3 4 5 6 7 8 9
//    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    *  |F|NRI|  Type   | a single NAL unit ... |
//    *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    */
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
//   ret = rtpSendPacket(socket, rtpChannel, rtpPacket, frameSize);
//   if (ret < 0)
//     return -1;

//   rtpPacket->rtpHeader.seq++;
//   sendBytes += ret;
//   if ((naluType & 0x1F) == 7 ||
//       (naluType & 0x1F) == 8) // 如果是SPS、PPS就不需要加时间戳
//     goto out;
// } else // nalu长度小于最大包场：分片模式
// {
//   /*
//    *  0                   1                   2
//    *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
//    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    * | FU indicator  |   FU header   |   FU payload   ...  |
//    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    */

//   /*
//    *     FU Indicator
//    *    0 1 2 3 4 5 6 7
//    *   +-+-+-+-+-+-+-+-+
//    *   |F|NRI|  Type   |
//    *   +---------------+
//    */

//   /*
//    *      FU Header
//    *    0 1 2 3 4 5 6 7
//    *   +-+-+-+-+-+-+-+-+
//    *   |S|E|R|  Type   |
//    *   +---------------+
//    */

//   int pktNum = frame_size / RTP_MAX_PKT_SIZE;        // 有几个完整的包
//   int remainPktSize = frame_size % RTP_MAX_PKT_SIZE; //
//   剩余不完整包的大小 int i, pos = 1;

//   /* 发送完整的包 */
//   for (i = 0; i < pktNum; i++) {
//     rtpPacket->payload[0] = (naluType & 0x60) | 28;
//     rtpPacket->payload[1] = naluType & 0x1F;

//     if (i == 0)                                     //第一包数据
//       rtpPacket->payload[1] |= 0x80;                // start
//     else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
//       rtpPacket->payload[1] |= 0x40;                // end

//     memcpy(rtpPacket->payload + 2, frame + pos, RTP_MAX_PKT_SIZE);
//     ret = rtpSendPacket(socket, rtpChannel, rtpPacket, RTP_MAX_PKT_SIZE +
//     2); if (ret < 0)
//       return -1;

//     rtpPacket->rtpHeader.seq++;
//     sendBytes += ret;
//     pos += RTP_MAX_PKT_SIZE;
//   }

//   /* 发送剩余的数据 */
//   if (remainPktSize > 0) {
//     rtpPacket->payload[0] = (naluType & 0x60) | 28;
//     rtpPacket->payload[1] = naluType & 0x1F;
//     rtpPacket->payload[1] |= 0x40; // end

//     memcpy(rtpPacket->payload + 2, frame + pos, remainPktSize + 2);
//     ret = rtpSendPacket(socket, rtpChannel, rtpPacket, remainPktSize +
//     2); if (ret < 0)
//       return -1;

//     rtpPacket->rtpHeader.seq++;
//     sendBytes += ret;
//   }
// }
// }
