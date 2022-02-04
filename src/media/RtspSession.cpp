#include "RtspSession.h"

#include "util/StringSplit.h"

#include <glog/logging.h>
#include <unordered_map>

static int getLineFromBuf(char *buf, char *line, int max_char) {
  if (!buf || !line || max_char <= 1) {
    return -1;
  }

  int index = 0;
  while (*buf != '\n' && index < max_char - 1) {
    *line = *buf;
    line++;
    buf++;
    index++;
  }

  *line = '\0';
  index++;

  return index;
}

RtspSession::RtspSession(TcpServer *serv, Dispatcher::Ptr &disp,
                         std::string &ip, int port, int fd, std::string name)
    : TcpConnect(serv, disp, ip, port, fd, name), reponse_seq_(0),
      rtp_channel_(-1), rtcp_channel_(-1) {}

RtspSession::~RtspSession() {}

int RtspSession::ReadHandler() {
  LOG(INFO) << "rtsp session read handler";

  static std::unordered_map<std::string, std::function<int(RtspSession *)>>
      s_method_table;
  static std::once_flag method_reg_flag;
  std::call_once(method_reg_flag, [&] {
    s_method_table.emplace(
        "OPTIONS", [](RtspSession *p) { return p->HandleCmd_OPTIONS(); });
    s_method_table.emplace(
        "DESCRIBE", [](RtspSession *p) { return p->HandleCmd_DESCRIBE(); });
    s_method_table.emplace("SETUP",
                           [](RtspSession *p) { return p->HandleCmd_SETUP(); });
    s_method_table.emplace("PLAY",
                           [](RtspSession *p) { return p->HandleCmd_PLAY(); });
  });

  std::string msg = buffer_reader_.GetStrUntilCRLFCRLF();
  if (!msg.size()) {
    LOG(ERROR) << "rtsp protocol error";
    return -1;
  }

  std::vector<StringPiece> pieces;
  StringSplitUseStrtok(msg.data(), pieces, "\r\n");

  // debug
  DLOG(INFO) << msg;
  for (auto &s : pieces) {
    DLOG(INFO) << s;
  }

  char method[40] = {0};
  char url[100] = {0};
  char version[40] = {0};

  if (pieces.empty()) {
    LOG(ERROR) << "split rtsp protocol fail";
  }

  if (sscanf(msg.data(), "%s %s %s", method, url, version) != 3) {
    LOG(ERROR) << "parse err";
    return -1;
  }

  LOG(INFO) << method << " " << url << " " << version;

  rtsp_url_ = url;
  rtsp_version_ = version;

  request_param_map_.clear();

  std::string key;
  std::string value;
  for (int i = 1; i < pieces.size(); i++) {
    auto &pie = pieces[i];
    bool second = false;
    while (pie.size()) {
      if (pie[0] == ' ') {
        pie.remove_prefix(1);
        continue;
      }

      if (pie[0] != ':') {
        if (!second) {
          key += pie[0];
        } else {
          value += pie[0];
        }
      } else {
        second = true;
      }

      pie.remove_prefix(1);
    }

    request_param_map_.emplace(std::move(key), std::move(value));
  }

  auto iter_Cseq = request_param_map_.find("CSeq");
  if (iter_Cseq == request_param_map_.end()) {
    LOG(ERROR) << "rtsp find CSeq fail";
    return -1;
  }

  reponse_seq_ = atoi(iter_Cseq->second.data());

  // debug
  for (auto &[k, v] : request_param_map_) {
    LOG(INFO) << "k:" << k << " v:" << v;
  }

  auto iter_metod = s_method_table.find(method);
  if (iter_metod == s_method_table.end()) {
  }

  if (iter_metod->second(this) < 0) {
    LOG(ERROR) << "method hanfler error";
    return -1;
  }

  return 1;
}

int RtspSession::HandleCmd_OPTIONS() {
  LOG(INFO) << "options";

  char buf[1024];
  memset(buf, 0, sizeof(buf));

  sprintf(buf,
          "RTSP/1.0 200 OK\r\n"
          "CSeq: %d\r\n"
          "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
          "\r\n",
          reponse_seq_);

  LOG(INFO) << "s->c  " << buf;
  buffer_writer_.Append(buf, strlen(buf));
  return 1;
}

int RtspSession::HandleCmd_DESCRIBE() {
  LOG(INFO) << "describe";

  char sdp_buf[1024];
  memset(sdp_buf, 0, sizeof(sdp_buf));
  char response_buf[2 * 1024];
  memset(response_buf, 0, sizeof(response_buf));

  std::string localip = GetRtspLocalIpFromUrl();

  snprintf(sdp_buf, sizeof(sdp_buf),
           "v=0\r\n"
           "o=- 9%ld 1 IN IP4 %s\r\n"
           "t=0 0\r\n"
           "a=control:*\r\n"
           "m=video 0 RTP/AVP 96\r\n"
           "a=rtpmap:96 H264/90000\r\n"
           "a=control:track0\r\n",
           time(NULL), localip.data());

  snprintf(response_buf, sizeof(response_buf),
           "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
           "Content-Base: %s\r\n"
           "Content-type: application/sdp\r\n"
           "Content-length: %d\r\n\r\n"
           "%s",
           reponse_seq_, rtsp_url_.data(), static_cast<int>(strlen(sdp_buf)),
           sdp_buf);

  buffer_writer_.Append(response_buf, strlen(response_buf));
  return 1;
}

int RtspSession::HandleCmd_SETUP() {
  LOG(INFO) << "setup";

  auto &transport_str = request_param_map_["Transport"];
  DLOG(INFO) << "transoort params:" << transport_str;

  std::vector<StringPiece> pieces;
  StringSplitUseStrtok(transport_str.data(), pieces, ";");

  for (auto &s : pieces) {
    DLOG(INFO) << s;
  }

  if (pieces.empty()) {
    return -1;
  }

  if (pieces[0] == "RTP/AVP/TCP") {
    int rtpChannel;
    int rtcpChannel;

    if (pieces.size() < 3) {
      return -1;
    }

    sscanf(pieces[2].data(), "interleaved=%d-%d", &rtpChannel, &rtcpChannel);
    LOG(INFO) << "interleved " << rtpChannel << " " << rtcpChannel;

    char response_buf[1024];
    memset(response_buf, 0, sizeof(response_buf));

    snprintf(response_buf, sizeof(response_buf),
             "RTSP/1.0 200 OK\r\n"
             "CSeq: %d\r\n"
             "Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n"
             "Session: 66334873\r\n"
             "\r\n",
             reponse_seq_, rtpChannel, rtcpChannel);

    buffer_writer_.Append(response_buf, strlen(response_buf));

  } else if (pieces[0] == "RTP/AVP/UDP") {

  } else {
    // return -1;
  }

  return 1;
}

int RtspSession::HandleCmd_PLAY() {
  LOG(INFO) << "play";

  char response_buf[1024];
  memset(response_buf, 0, sizeof(response_buf));

  sprintf(response_buf,
          "RTSP/1.0 200 OK\r\n"
          "CSeq: %d\r\n"
          "Range: npt=0.000-\r\n"
          "Session: 66334873; timeout=60\r\n\r\n",
          reponse_seq_);
  
  buffer_writer_.Append(response_buf,strlen(response_buf));

  return 1;
}

std::string RtspSession::GetRtspLocalIpFromUrl() {
  // std::string result;
  int begin_len = strlen("rtsp://");
  if (memcmp(rtsp_url_.data(), "rtsp://", begin_len)) {
    return std::string();
  }

  std::string result;
  for (int i = begin_len; i < rtsp_url_.size(); i++) {
    if (rtsp_url_[i] == ':') {
      return result;
    }
    result += rtsp_url_[i];
  }

  return std::string();
}
