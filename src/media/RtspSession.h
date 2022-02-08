#pragma once

#include "network/TcpConnect.h"
#include "network/TimerController.h"

#include <map>

class RtpSession;
class RtspServer;

enum RtspState : int { RtspStateInit, RtspStatePause, RtspStatePlay };

class RtspSession : public TcpConnect {
public:
  using Ptr = std::shared_ptr<RtspSession>;

  RtspSession(TcpServer *serv, Dispatcher::Ptr &disp, std::string &ip, int port,
              int fd, std::string name);
  ~RtspSession();

  int ReadHandler();
  RtspState GetState() { return state_; }
  RtspServer *GetRtspServer();

private:
  int HandleRtsp();
  int HandleRtcp();

  int HandleCmd_OPTIONS();
  int HandleCmd_DESCRIBE();
  int HandleCmd_SETUP();
  int HandleCmd_PLAY();

  std::string GetRtspLocalIpFromUrl();
  std::string GetRtspUrlSuffixFromUrl();

  std::string rtsp_url_;
  std::string rtsp_version_;
  std::string rtsp_url_suffix_;

  std::map<std::string, std::string> request_param_map_;
  int reponse_seq_;
  int rtp_channel_;
  int rtcp_channel_;

  RtspState state_;
  std::shared_ptr<RtpSession> rtp_sess_;
};
