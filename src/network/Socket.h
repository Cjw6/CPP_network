//
// Created by cjw on 21-2-12.
//

#ifndef DEMO_SOCKET_H
#define DEMO_SOCKET_H

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <netdb.h>
#include <netinet/tcp.h> /* for TCP_XXX defines */
#include <resolv.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define INVALID_SOCKET -1

using SocketOptions = std::function<void(int sock)>;
using BindOrConnect = std::function<bool(int sock, void *data, int listen_num)>;

int CreateSocket(std::string &host, int port, int socket_flags, int listen_num,
                 SocketOptions socket_options, BindOrConnect bind_or_connect);

bool BindListenSocket(int sock, void *data, int listen_num);

bool ConnectSocket(int sock, void *data, int listen_num);

int CreateServerSocket(std::string ip, int port, int socket_flags,
                       int listen_num);

int CreateClientSocket(std::string ip,int port,int socket_flags);                       

int SetSocketNonBlocking(int fd);

void SetSocketNodelay(int fd);

void SetSocketNoLinger(int fd);


#endif // DEMO_SOCKET_H
