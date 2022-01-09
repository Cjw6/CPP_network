#pragma once

// #include "network/BufferWriter.h"
#include <vector>
#include <memory>
#include <deque>

class CByteBuffer {
public:
  CByteBuffer();
  ~CByteBuffer();

  char *Begin();
  char *ReadBegin();
  char *WriteBegin();

  void Retrieve(int nLen);
  void RetrieveAll();
  void Append(const char *data, int len);

  int DataLen();
  int ReadavbleSize();
  int WritableSize();
  void DebugSpace();

private:
  // forbidden copy
  CByteBuffer(CByteBuffer &)=delete;
  CByteBuffer &operator=(CByteBuffer &)=delete;

  std::vector<char> m_buf;
  int m_nReadIndex;
  int m_nWriteIndex;
};


using BufferReader=CByteBuffer;

class BufFixedBlock
{
public:
	// BufFixedBlock(int nLen);
  BufFixedBlock(const char *data, int len);
	~BufFixedBlock();
	const char *Data();
	const char *Header();
	int Len();
	int WaitSendLen();
	void Retrieve(int len);
	bool Finish();

private:
	char *m_pBuf;
	int m_nLen;
	int m_index;
};

using BufFixedBlkPtr=std::shared_ptr<BufFixedBlock>;

class CBufWriter
{
public:
	CBufWriter();
	~CBufWriter();
	void Append(BufFixedBlkPtr &pBuf);
	int Send(int sockfd);
	int QueueSize();

private:
	std::deque<BufFixedBlkPtr> m_buf;
};

using BufferWriter=CBufWriter;
