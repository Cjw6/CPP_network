#include "ByteBuffer.h"
#include "SafeMem.h"

#include <cstdlib>
#include <cstring>

#include <glog/logging.h>

const int ByteBuffer::kDefaultSize = 0;

ByteBuffer::ByteBuffer() : m_nReadIndex(0), m_nWriteIndex(0) {
  if (kDefaultSize) {
    m_buf = (char *)std::malloc(kDefaultSize);
    if (!m_buf) {
      m_capcity = 0;
    } else {
      m_capcity = kDefaultSize;
    }
  } else {
    m_buf = nullptr;
    m_capcity = 0;
  }
}

ByteBuffer::~ByteBuffer() { SafeFree(m_buf); }

char *ByteBuffer::Begin() { return m_buf; }

char *ByteBuffer::ReadBegin() { return m_buf + m_nReadIndex; }

char *ByteBuffer::WriteBegin() { return m_buf + m_nWriteIndex; }

char *ByteBuffer::End() { return m_buf + m_capcity; }

std::string ByteBuffer::GetStrUntilCRLF() {
  const char *tail = strstr(ReadBegin(), "\r\n");
  if (!tail) {
    return std::string();
  }

  int size = tail - ReadBegin();
  std::string str(ReadBegin(), size);

  return str;
}

std::string ByteBuffer::GetStrUntilCRLFCRLF() {
  const char *tail = strstr(ReadBegin(), "\r\n\r\n");
  if (!tail) {
    return std::string();
  }

  int size = tail - ReadBegin();
  std::string str(ReadBegin(), size);
  Retrieve(size + 4);
  return str;
}

void ByteBuffer::Retrieve(int nLen) {
  m_nReadIndex += nLen;
  CHECK(m_nReadIndex <= m_nWriteIndex) << " CByteBuffer  overflow write...";
  if (m_nReadIndex == m_nWriteIndex) {
    m_nReadIndex = 0;
    m_nWriteIndex = 0;
  }
}

void ByteBuffer::RetrieveAll() {
  m_nReadIndex = 0;
  m_nWriteIndex = 0;
}

void ByteBuffer::ResetBegin() {
  std::copy(ReadBegin(), WriteBegin(), Begin());
  m_nWriteIndex = WritableSize();
  m_nReadIndex = 0;
}

void ByteBuffer::Append(const char *data, int len) {
  if (Capcity() >= m_nWriteIndex + len) {
    memcpy(WriteBegin(), data, len);
  } else {
    int target_len = m_nWriteIndex + len;
    int new_size = std::max(m_capcity * 2, target_len);
    if (new_size == target_len) {
      Resize(new_size * 2);
    } else {
      Resize(new_size);
    }
    memcpy(WriteBegin(), data, len);
  }
  m_nWriteIndex += len;
}

int ByteBuffer::ReadableSize() { return m_nWriteIndex - m_nReadIndex; }

int ByteBuffer::WritableSize() { return Capcity() - m_nWriteIndex; }

int ByteBuffer::Size() { return m_nWriteIndex; }

int ByteBuffer::Capcity() { return m_capcity; }

int ByteBuffer::Resize(int n) {
  if (m_capcity >= n) {
    return 1;
  }

  // LOG(INFO) << " resize bytebuffer";

  char *new_buf = (char *)realloc(m_buf, n);
  CHECK(new_buf) << " ByteBuffer realloc fail";
  m_capcity = n;
  m_buf = new_buf;
  memset(WriteBegin(), 0, WritableSize());
  return 1;
}

void ByteBuffer::SetReadIndex(int n) { m_nReadIndex = n; }

void ByteBuffer::SetWriteIndex(int n) { m_nWriteIndex = n; }
