#include "media/RtpSession.h"
#include "util/Log.h"

RtpSession::RtpSession(TcpConnect::Ptr tcp_conn, int rtp_socket)
    : tcp_conn_(tcp_conn), rtp_socket_(-1), play_(false),
      has_key_frame_(false) {}

RtpSession::~RtpSession() { LOG(INFO) << "rtp session destruct ...."; }

void RtpSession::SendRtpPacket(char *frame, int len, bool key_frame) {
  if (play_) {
    // LOG(INFO) << "send rtp packet ...1";
    if (!has_key_frame_ && key_frame) {
      // LOG(INFO) << "first key frame"<<key_frame;
      has_key_frame_ = true;
    }
    if (has_key_frame_) {
      // LOG(INFO) << "sending rtp packet ...2";
      auto tcp = tcp_conn_.lock();
      if (tcp) {
        SendBufBlock::Ptr buf = std::make_shared<SendBufBlock>(frame, len);
        auto self = shared_from_this();
        tcp->disp_->RunTask([buf, self]() {
          auto tcp = self->tcp_conn_.lock();
          if (!tcp) {
            return;
          }
          // LOG(INFO) << "rtp send ...";
          tcp->Send(buf);
        });
      }
    }
  }
}

void RtpSession::EnablePlay(bool res) {
  if (res) {
    play_ = res;
  } else {
    play_ = res;
    has_key_frame_ = false;
  }
}

bool RtpSession::IsPlay() { return play_; }
