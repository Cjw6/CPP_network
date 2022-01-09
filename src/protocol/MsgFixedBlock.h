#pragma once 

class MsgBuf;

class MsgFixedBlock {
public:
  MsgFixedBlock(const char *data, int len);
  MsgFixedBlock(MsgBuf &bufBlk);
  ~MsgFixedBlock();
  const char *Data();
  int Len();
  
private:
  char *m_pBuf;
  int m_nLen;
};
