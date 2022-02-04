#pragma once

#include "network/TcpConnect.h"

#include <map>

class RtspSession : public TcpConnect {
public:
  using Ptr = std::shared_ptr<RtspSession>;

  RtspSession(TcpServer *serv, Dispatcher::Ptr &disp, std::string &ip, int port,
              int fd, std::string name);
  ~RtspSession();

  int ReadHandler();

private:
  int HandleCmd_OPTIONS();
  int HandleCmd_DESCRIBE();
  int HandleCmd_SETUP();
  int HandleCmd_PLAY();

  std::string GetRtspLocalIpFromUrl();

  std::string rtsp_url_;
  std::string rtsp_version_;
  std::map<std::string, std::string> request_param_map_;
  int reponse_seq_;
  int rtp_channel_;
  int rtcp_channel_;
};
