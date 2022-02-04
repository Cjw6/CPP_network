#pragma once

// #include "network/BufferWriter.h"
#include <deque>
#include <memory>
#include <vector>

#include "util/ByteBuffer.h"
#include "util/StringPiece.h"

class SendBufBlock {
public:
  SendBufBlock();
  SendBufBlock(const char *data, int len);
  ~SendBufBlock();

  SendBufBlock(SendBufBlock &blk) = delete;
  SendBufBlock &operator=(SendBufBlock &blk) = delete;

  SendBufBlock(SendBufBlock &&blk);
  SendBufBlock &operator=(SendBufBlock &&blk);

  const char *Data();
  const char *Header();
  int Len();
  int WaitSendLen();
  void Retrieve(int len);
  bool Finish();
  void Append(const char *data, int len);
  void Clear();
  std::string ToString();

private:
  char *m_pBuf;
  int m_nLen;
  int m_index;
  int m_capcity;
};

// using BufFixedBlkPtr=std::shared_ptr<BufBlock>;

class WriteSendBufList {
public:
  WriteSendBufList();
  ~WriteSendBufList();
  void Append(const char *pdata, int len);
  void Append(SendBufBlock &blk);
  void AppendMoveBuf(SendBufBlock &blk);
  int Send(int sockfd);
  int QueueSize();

private:
  std::deque<SendBufBlock> m_buf;
};

using BufferReader = ByteBuffer;
using BufferWriter = WriteSendBufList;
