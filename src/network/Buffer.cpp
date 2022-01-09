#include "Buffer.h"

#include <cassert>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

CByteBuffer::CByteBuffer() : m_nReadIndex(0), m_nWriteIndex(0) {
  // m_buf.reserve(1024);
}

CByteBuffer::~CByteBuffer() {}

char *CByteBuffer::Begin() { return m_buf.data(); }

char *CByteBuffer::ReadBegin() { return m_buf.data() + m_nReadIndex; }

char *CByteBuffer::WriteBegin() { return m_buf.data() + m_nWriteIndex; }

void CByteBuffer::Retrieve(int nLen) {
  m_nReadIndex += nLen;
  if (m_nReadIndex == m_nWriteIndex) {
    m_nReadIndex = 0;
    m_nWriteIndex = 0;
  }
}

void CByteBuffer::RetrieveAll() {
  m_nReadIndex = 0;
  m_nWriteIndex = 0;
}

int CByteBuffer::DataLen() { return m_nWriteIndex - m_nReadIndex; }

void CByteBuffer::Append(const char *data, int len) {
  // LOG_DEBUG("ri:%d wI:%d size:%d\n", m_nReadIndex, m_nWriteIndex,
  // m_buf.size());
  m_buf.insert(m_buf.begin() + m_nWriteIndex, data, data + len);
  m_nWriteIndex += len;
}

int CByteBuffer::ReadavbleSize() { return m_nWriteIndex - m_nReadIndex; }

int CByteBuffer::WritableSize() { return m_buf.size() - m_nWriteIndex; }

void CByteBuffer::DebugSpace() {
  // printf("Debug space  size:%d  capcity%d\n  ri:%d  wi%d\n", m_buf.size(),
  // m_buf.capacity(), m_nReadIndex, m_nWriteIndex);
}

CBufWriter::CBufWriter() {}

CBufWriter::~CBufWriter() {}

void CBufWriter::Append(BufFixedBlkPtr &pBuf) { m_buf.push_back(pBuf); }

// BufFixedBlock::BufFixedBlock(int nLen) {
//   // LOG_DEBUG("new blk %d \n", nLen);
//   m_nLen = nLen;
//   m_pBuf = new char[m_nLen];
// }

BufFixedBlock::BufFixedBlock(const char *data, int len)
    : m_pBuf(nullptr), m_nLen(0) {
  assert(data);
  assert(len > 0);
  m_pBuf = new char[len];
  memcpy(m_pBuf, data, m_nLen);
}

BufFixedBlock::~BufFixedBlock() {
  // LOG_DEBUG("destruct .....\n");
  if (m_pBuf) {
    delete[] m_pBuf;
  }
}

int BufFixedBlock::WaitSendLen() { return m_nLen - m_index; }

void BufFixedBlock::Retrieve(int len) {
  m_index += len;
  // LOG_DEBUG("index  %d , size: %d  len %d\n", m_index, m_nLen, m_index);
  assert(m_index <= m_nLen);
}

bool BufFixedBlock::Finish() { return m_index == m_nLen; }

const char *BufFixedBlock::Header() {
  // LOG_DEBUG("index  %d , size: %d\n", m_index, m_nLen);
  return m_pBuf + m_index;
}

const char *BufFixedBlock::Data() { return m_pBuf; }

int BufFixedBlock::Len() { return m_nLen; }

int CBufWriter::Send(int sockfd) {
  int ret;
  do {
    if (m_buf.empty()) {
      return 0;
    }

    BufFixedBlkPtr &blk = m_buf.front();
    ret = ::send(sockfd, blk->Header(), blk->WaitSendLen(), 0);
    if (ret > 0) {
      blk->Retrieve(ret);
      if (blk->Finish()) {
        m_buf.pop_front();
      }
    } else if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN) {
        ret = 0;
      } else {
        // LOG_WARNING("send fail %s \n", strerror(errno));
        ret = -1;
      }
    } else {
      ret = -1;
    }
  } while (ret > 0);
  // LOG_DEBUG("sess write  return value %d \n", ret);
  return ret;
}

int CBufWriter::QueueSize() { return m_buf.size(); }
