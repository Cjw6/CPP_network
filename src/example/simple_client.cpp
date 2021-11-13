#include "network/Socket.h"
#include <cstdio>
#include <glog/logging.h>
int main(){
  int fd=CreateClientSocket("0.0.0.0", 8888, 0);
  LOG(INFO)<<fd;
  getchar();
  return 0;
}