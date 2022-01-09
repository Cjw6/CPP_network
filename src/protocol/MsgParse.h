#pragma once 

#include "MsgBuf.h"
#include "MsgBufSlice.h"
#include "MsgCodecType.h"
#include <map>

// class MsgBufSlice;

class MsgUnitValue {
public:
	MsgUnitValue();
  MsgUnitValue(MsgCodecType type,MsgBufSlice v);
  MsgCodecType type;
	MsgBufSlice v;
};

class MsgParse {
public:
  explicit MsgParse(MsgBufSlice msg_buf);

  int32_t ParseHeader(uint32_t &cmd1, uint32_t &cmd2);
  bool ParseUnit(MsgBufSlice &key, MsgBufSlice &value, MsgCodecType &type);
	bool ParseAllUnit();
  bool ParseEnd();
  static bool CheckAdler32(MsgBufSlice &msg_buf);

  uint32_t GetElemNum() { return elem_num_; }
  uint32_t GetDataLen() { return data_len_; }

  static void ParseArray(MsgBufSlice &tag, MsgBufSliceArray &msgSlices);
  static const char *FindMsgHeader(MsgBufSlice &msgSlice);
  static int SplitBufToMsgBufBlockList(MsgBufSlice bufSlice,
                                       MsgBufList &listBufBlks);

private:
  char *BufBegin();
  void Retrieve(uint32_t len);

  MsgBufSlice buf_;
  uint32_t elem_num_;
  uint32_t data_len_;
  uint32_t parse_index_;
	std::map<std::string,MsgUnitValue>  msg_map_;
};


