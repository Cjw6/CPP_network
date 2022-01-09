//
// Created by cjw on 21-2-12.
//

#include "Socket.h"
#include <fcntl.h>
#include <glog/logging.h>
#include <unistd.h>

int CreateSocket(std::string &host, int port, int socket_flags, int listen_num,
                 SocketOptions socket_options, BindOrConnect bind_or_connect) {
  addrinfo hints;
  addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family =
      AF_UNSPEC; // AF_UNSPEC则意味着函数返回的是适用于指定主机名和服务名且适合任何协议族的地址。
  hints.ai_socktype = SOCK_STREAM; // tcp
  hints.ai_flags = socket_flags;
  hints.ai_protocol = 0;

  auto service = std::to_string(port);

  if (getaddrinfo(host.c_str(), service.c_str(), &hints, &result)) {
    res_init();
    return -1;
  }
  for (auto rp = result; rp; rp = rp->ai_next) {
    // Create a socket
    auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if (sock == INVALID_SOCKET) {
      continue;
    }

    if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
      continue;
    }

    int yes = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&yes),
               sizeof(yes));

    //这里选择 自定义的 socket option

    if (rp->ai_family == AF_INET6) {
      int no = 0;
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char *>(&no),
                 sizeof(no));
    }

    struct linger so_linger;
    so_linger.l_onoff = true;
    so_linger.l_linger = 30;
    setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<char *>(&yes),
               sizeof(yes));

    if (socket_options)
      socket_options(sock);

    // bind or connect
    if (bind_or_connect)
      if (bind_or_connect(sock, rp, listen_num)) {
        freeaddrinfo(result);
        return sock;
      }

    close(sock);
  }

  freeaddrinfo(result);
  return INVALID_SOCKET;
}

bool BindListenSocket(int sock, void *data, int listen_num) {
  addrinfo *ai = reinterpret_cast<addrinfo *>(data);
  if (::bind(sock, ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen))) {
    return false;
  }
  if (::listen(sock, listen_num)) { // Listen through 5 channels
    return false;
  }
  return true;
}

bool ConnectSocket(int sock, void *data, int listen_num) {
  addrinfo *ai = reinterpret_cast<addrinfo *>(data);
  if (connect(sock, ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen)) < 0) {
    LOG(ERROR) << "connect error: " << strerror(errno) << " errno: " << errno;
    return false;
  }
  return true;
}

int CreateServerSocket(std::string ip, int port, int socket_flags,
                       int listen_num) {
  int sock_fd = CreateSocket(ip, port, socket_flags, listen_num, nullptr,
                             BindListenSocket);
  if (sock_fd < 0) {
    LOG(ERROR) << "create server sockset fail";
    return -1;
  }
  return sock_fd;
}

int CreateClientSocket(std::string ip, int port, int socket_flags) {
  int sock_fd = CreateSocket(ip, port, socket_flags, 0, nullptr, ConnectSocket);
  if (sock_fd < 0) {
    LOG(ERROR) << "create server sockset fail";
    return -1;
  }
  return sock_fd;
}

// int CreateClientConnectSocket(std::string ip, int port, int socket_flag) {
//   int sockfd;
//   struct sockaddr_in servaddr;

//   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//     LOG(ERROR) << "create socket error:" << strerror(errno)
//                << " errno: " << errno;
//     return -1;
//   }

//   memset(&servaddr, 0, sizeof(servaddr));
//   servaddr.sin_family = AF_INET;
//   servaddr.sin_port = htons(port);

//   if (inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr) <= 0) {
//     LOG(ERROR) << "inet_pton error for" << ip;
//     return -2;
//   }

//   if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
//     LOG(ERROR) << "connect error: " << strerror(errno) << " errno: " <<
//     errno; return -3;
//   }
//   return sockfd;
// }

int SetSocketNonBlocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0);
  if (flag == -1)
    return -1;

  flag |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) == -1)
    return -1;
  return 0;
}

void SetSocketNodelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

void SetSocketNoLinger(int fd) {
  struct linger linger_;
  linger_.l_onoff = 1;
  linger_.l_linger = 30;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_,
             sizeof(linger_));
}