#pragma once 

#include <vector>
#include <cstdint>
#include <list>

class MsgBuf : public std::vector<char> {
public:
  MsgBuf() = default;
  MsgBuf(const char *data, int len);
  void Append(const char *data, int len);
  void PutFixedUint8(uint8_t value);
  void PutFixedUint32(uint32_t value);
  void PutFixedUint64(uint64_t value);
};

using MsgBufList=std::list<MsgBuf>;

