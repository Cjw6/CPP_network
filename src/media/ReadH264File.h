#pragma once

#include "util/ByteBuffer.h"
#include <cstdio>
#include <string>

class ReadH264File {
public:
  ReadH264File();
  ~ReadH264File();
  int Open(std::string &path);
  int ReadFrame(ByteBuffer &piece, int size);

private:
  static int startCode3(char *buf);
  static int startCode4(char *buf);
  FILE *fp_;
};
