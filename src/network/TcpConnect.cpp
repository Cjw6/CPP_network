#include "TcpConnect.h"
#include "Buffer.h"
#include "Channels.h"
#include "TcpServer.h"
#include <asm-generic/errno-base.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <mutex>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

TcpConnect::TcpConnect(TcpServer *serv, Dispatcher::Ptr &disp, std::string ip,
                       int port, int fd, std::string name)
    : Channels(disp, fd), serv_(serv), ip_(ip), port_(port), name_(name),
      read_cb_(nullptr), error_cb_(nullptr), connected_(true) {
  if (fd > 0) {
    UpdateEventOption(kEventAdd, EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  }
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

TcpConnect::~TcpConnect() {
  LOG(INFO) << "destruct..." << GetFd() << " " << name_;
}

void TcpConnect::HandleEvents() {
  disp_->ExecTask([this] {
    if (events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      if (HandleRead() <= 0) {
        HandleError();
        return;
      }
    }

    if (events_ & EPOLLOUT) {
      if (HandleWrite() <= 0) {
        HandleError();
        return;
      }
    }

    if (events_ & EPOLLERR) {
      HandleError();
      return;
    }
  });
}

int TcpConnect::HandleRead() {
  do {
    char buf[1024] = {0};
    int n = read(fd_, buf, 1024);
    if (n > 0) {
      buffer_reader_.Append(buf, n);
    } else if (n < 0) {
      if (errno == EAGAIN || errno == EINTR) {
        break;
      } else {
        return -1;
      }
    } else {
      LOG(WARNING) << "disconnect ";
      return 0;
    }
  } while (1);

  CHECK(read_cb_) << "read_cb is null";
  read_cb_(shared_from_this(), buffer_reader_, buffer_writer_);

  if (buffer_writer_.QueueSize()) {
    if (buffer_writer_.Send(fd_) < 0) {
      return -1;
    }
  }

  if (buffer_writer_.QueueSize() > 0) {
    UpdateEventOption(kEventMod,
                      EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  } else {
    UpdateEventOption(kEventMod, EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  }

  return 1;
}

int TcpConnect::HandleWrite() {
  if (buffer_writer_.Send(fd_) < 0) {
    return -1;
  }

  if (buffer_writer_.QueueSize() > 0) {
    UpdateEventOption(kEventMod,
                      EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  } else {
    UpdateEventOption(kEventMod, EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  }

  return 1;
}

int TcpConnect::HandleError() {
  auto self = shared_from_this();
  PLOG(WARNING) << "tcp conn handle error";

  if (error_cb_)
    error_cb_(shared_from_this());

  UpdateEventOption(kEventDel,
                    EPOLLIN | EPOLLOUT | EPOLLET | EPOLLONESHOT | EPOLLRDHUP);
  close(fd_);

  connected_ = false;
  serv_->RemoveSocketByName(GetName());

  return -1;
}
