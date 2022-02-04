#pragma once

#include <string>
#include <vector>

class ByteBuffer {
public:
  ByteBuffer();
  ~ByteBuffer();

  char *Begin();      //数据起始位置
  char *ReadBegin();  //可读数据起始位置
  char *WriteBegin(); //可写数据起始位置
  char *End();

  std::string GetStrUntilCRLF();
  std::string GetStrUntilCRLFCRLF();

  void Retrieve(int nLen);
  void RetrieveAll();
  void ResetBegin();

  void Append(const char *data, int len);
  
  int ReadableSize();
  int WritableSize();
  int Capcity();      // vector capcity
  int Resize(int n);

  void SetReadIndex(int n);
  void SetWriteIndex(int n);
  
  static const int kDefaultSize;

private:
  // forbidden copy
  ByteBuffer(ByteBuffer &) = delete;
  ByteBuffer &operator=(ByteBuffer &) = delete;

  char *m_buf;
  int m_nReadIndex;
  int m_nWriteIndex;
  int m_capcity;
};