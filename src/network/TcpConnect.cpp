#include "TcpConnect.h"
#include "Channels.h"
#include "TcpServer.h"
#include <cstring>
#include <errno.h>
#include <glog/logging.h>
#include <mutex>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

TcpConnect::TcpConnect(TcpServer *serv, Dispatcher::Ptr &disp, std::string ip,
                       int port, int fd, std::string name)
    : Channels(disp, fd), serv_(serv), ip_(ip), port_(port), name_(name),
      read_cb_(nullptr), error_cb_(nullptr) {
  memset(buffer, 0, 512);
  if (fd > 0) {
    UpdateEventOption(kEventAdd, EPOLLIN);
  }
}

TcpConnect::~TcpConnect() {
  LOG(INFO)<<"destruct..."<<GetFd()<<" "<<name_;
}

int TcpConnect::HandleRead() {
  LOG(INFO)<<"tcp conn handle read ";
  int n = read(GetFd(), buffer, 512);
  if (n > 0) {
    LOG(INFO) << "read " << n << " " << buffer;
    if (read_cb_) {
      index_ = n;
      return read_cb_(shared_from_this(),buffer, index_);
    }else{
      LOG(FATAL)<<"read_cb_ is null";
    }
  } else if (n == 0) {
    LOG(WARNING) << "tcp disconnect..";
    return -1;
  } else {
    PLOG(ERROR) << "read error...";
    if (errno == EAGAIN)
      return 1;
  }
  return -1;
}

int TcpConnect::HandleWrite() { 
  LOG(INFO)<<"tcp conn handle write ";
  return 1; 
}

int TcpConnect::HandleError() {
  LOG(WARNING)<<"tcp conn handle error";
  auto self=shared_from_this();
  if (error_cb_)
    error_cb_(shared_from_this());
  close(GetFd());
  UpdateEventOption(kEventDel, EPOLLIN | EPOLLOUT);
  serv_->RemoveSocketByName(name_);
  return -1;
}

int TcpConnect::Send(int n) {
  int res = write(GetFd(), buffer, n);
  if (res > 0) {
    LOG(INFO) << "write " << res << " " << buffer;
  } else if (res == 0) {
    LOG(INFO) << "tcp disconnect...";
    return -1;
  } else {
    PLOG(ERROR) << "write error...";
    if (errno == EAGAIN)
      return 1;
    return -1;
  }
  return 1;
}