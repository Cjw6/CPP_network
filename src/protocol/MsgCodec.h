#pragma once 

// #include "MsgBuf.h"
// #include "MsgBufSlice.h"
#include "MsgCodecType.h"
#include <vector>

class MsgBuf;
class MsgBufSlice;

class MsgCodec {
public:
  MsgCodec(MsgBuf *buf_blk);
  ~MsgCodec();

  void StartGen(uint32_t cmd1 = 0, uint32_t cmd2 = 0);
  void PutKVUInt32(MsgBufSlice k, uint32_t v);
  void PutKVString(MsgBufSlice k, MsgBufSlice v);
  void PutArray(MsgBufSlice k, std::vector<MsgBufSlice> &buf_blks,
                MsgCodecType type);
  void FinshGen();

  uint32_t GetElemNum() { return elem_num_; }

  uint32_t GetDataLen() { return data_len_; }

  static uint32_t reserve_len;
  static char started_code[4];

private:
  MsgBuf *buf_;
  uint32_t elem_num_;
  uint32_t data_len_; //从头开始计算
};