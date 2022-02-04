#pragma once 

// #include "BufFixedBlock.h"
#include "MsgBuf.h"

#include <string>

class SendBufBlock;
class MsgBuf;

class MsgBufSlice {
public:
  MsgBufSlice();
	// BufBlockSlice(BufBlockSlice& b)=default;
  MsgBufSlice(MsgBuf &k);
  MsgBufSlice(std::string &str);
  MsgBufSlice(const char *data, uint32_t len);
  MsgBufSlice(const char *data);
  std::string ToString() { return std::string(data); }
  // BufFixedBlock ToBufFixedBlk(); //{ return BufFixedBlock(data, size); }

  const char *data;
  uint32_t size;
};


using MsgBufSliceArray= std::vector<MsgBufSlice> ;
