#include "Acceptor.h"
#include "Channels.h"
#include "Socket.h"
#include <cstddef>
#include <glog/logging.h>
#include <sys/epoll.h>

Acceptor::Acceptor(Dispatcher::Ptr &disp, std::string ip, int port,
                   int listen_num, int fd)
    : Channels(disp, fd), ip_(ip), port_(port), accept_cb_(nullptr) {
  // int fd = CreateServerSocket(ip_, port_, 0, listen_num);
  if (fd > 0) {
    UpdateEventOption(Channels::kEventAdd, EPOLLIN);
  }
}

Acceptor::~Acceptor() {}

void Acceptor::SetNewConnCb(AcceptNewConnectCallback cb) { accept_cb_ = cb; }

int Acceptor::HandleRead() {
  struct sockaddr_in cliaddr;
  memset(&cliaddr, 0, sizeof(cliaddr));
  socklen_t cliaddrlen=sizeof(cliaddr);

  int cli_fd = accept(GetFd(), (struct sockaddr *)&cliaddr, &cliaddrlen);
  if (cli_fd == -1) {
    PLOG(ERROR) << "accpet error:";
    return -1;
  }
  std::string cli_ip = inet_ntoa(cliaddr.sin_addr);
  int cli_port = cliaddr.sin_port;
  if (accept_cb_) {
    accept_cb_(cli_fd, cli_ip, cli_port);
    return 1;
  } else {
    LOG(ERROR) << "accept new conn callback is null";
    close(cli_fd);
    return -1;
  }
}
