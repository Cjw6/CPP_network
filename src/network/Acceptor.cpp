#include "Acceptor.h"
#include "Channels.h"
#include "Socket.h"
#include <cstddef>
#include <glog/logging.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>

#define ACCEPT_USE_EPOLLLT 0
#define ACCEPT_USE_EPOLLET 1

Acceptor::Acceptor(Dispatcher::Ptr &disp, std::string ip, int port,
                   int listen_num)
    : Channels(disp), ip_(ip), port_(port), accept_cb_(nullptr),
      listen_num_(listen_num), listem_fd_(-1) {}

Acceptor::~Acceptor() {}

int Acceptor::Listen() {
  listem_fd_ = CreateServerSocket(ip_, port_, 0, listen_num_);
  if (listem_fd_ < 0) {
    LOG(ERROR) << " listen error";
    return -1;
  }
  SetSocketNonBlocking(listem_fd_);
  UpdateEventOption(listem_fd_, kEventAdd, EPOLLIN | EPOLLET | EPOLLONESHOT);
  return 1;
}

void Acceptor::SetNewConnCb(AcceptNewConnectCallback cb) { accept_cb_ = cb; }

void Acceptor::HandleEvents() {
  // in_event_.store(true,std::memory_order_release);
  if (events_ & EPOLLIN) {
    if (HandleRead() < 0) {
      // in_event_.store(false,std::memory_order_release);
      HandleError();
      return;
    }
  }
  if (events_ & EPOLLERR) {
    HandleError();
    return;
  }
}

int Acceptor::HandleRead() {
#if ACCEPT_USE_EPOLLLT
  int cli_fd = 0;
  do {
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t cliaddrlen = sizeof(cliaddr);
    int cli_fd = accept(GetFd(), (struct sockaddr *)&cliaddr, &cliaddrlen);
    if (cli_fd == -1) {
      PLOG(ERROR) << "accpet error:";
    } else {
      std::string cli_ip = inet_ntoa(cliaddr.sin_addr);
      int cli_port = cliaddr.sin_port;
      if (accept_cb_) {
        accept_cb_(cli_fd, cli_ip, cli_port);
      } else {
        LOG(ERROR) << "accept new conn callback is null";
        close(cli_fd);
      }
    }
  } while (cli_fd > 0);
  UpdateEventOption(kEventAdd, EPOLLIN);
#elif ACCEPT_USE_EPOLLET
  int cli_fd = 0;
  struct sockaddr_in cliaddr;
  socklen_t cliaddrlen = sizeof(cliaddr);
  do {
    memset(&cliaddr, 0, sizeof(cliaddr));
    int cli_fd = accept(GetFd(), (struct sockaddr *)&cliaddr, &cliaddrlen);
    if (cli_fd <= 0) {
      PLOG(ERROR) << "accpet error:";
    } else {
      std::string cli_ip = inet_ntoa(cliaddr.sin_addr);
      int cli_port = cliaddr.sin_port;
      if (accept_cb_) {
        accept_cb_(cli_fd, cli_ip, cli_port);
      } else {
        LOG(ERROR) << "accept new conn callback is null";
        close(cli_fd);
      }
    }
  } while (cli_fd > 0);
  UpdateEventOption(listem_fd_, kEventMod, EPOLLIN | EPOLLET | EPOLLONESHOT);
#endif

  return 0;
}

void Acceptor::HandleError() {}
