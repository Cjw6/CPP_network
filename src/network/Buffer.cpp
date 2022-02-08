#include "Buffer.h"
#include "util/SafeMem.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <errno.h>
#include <memory>
#include <string.h>

#include <algorithm>
#include <cstring>

#include <glog/logging.h>

// static const char kCRLF[] = "\r\n";
// static const char kCRLFCRLF[] = "\r\n\r\n";

SendBufBlock::SendBufBlock()
    : m_pBuf(nullptr), m_nLen(0), m_index(0), m_capcity(0) {}

SendBufBlock::SendBufBlock(const char *data, int len)
    : m_pBuf(nullptr), m_nLen(0), m_index(0) {
  assert(data);
  assert(len > 0);
  m_pBuf = (char *)malloc(len);
  m_nLen = len;
  memcpy(m_pBuf, data, m_nLen);
  m_capcity = m_nLen;
}

SendBufBlock::~SendBufBlock() { SafeFree(m_pBuf); }

SendBufBlock::SendBufBlock(SendBufBlock &&blk) {
  SafeFree(this->m_pBuf);
  this->m_pBuf = blk.m_pBuf;
  blk.m_pBuf = nullptr;

  this->m_index = blk.m_index;
  this->m_capcity = blk.m_capcity;
  this->m_nLen = blk.m_nLen;

  blk.Clear();
}

SendBufBlock &SendBufBlock::operator=(SendBufBlock &&blk) {
  if (&blk != this) {
    SafeFree(this->m_pBuf);
    this->m_pBuf = blk.m_pBuf;
    blk.m_pBuf = nullptr;

    this->m_index = blk.m_index;
    this->m_capcity = blk.m_capcity;
    this->m_nLen = blk.m_nLen;

    blk.Clear();
  }
  return *this;
}

int SendBufBlock::WaitSendLen() { return m_nLen - m_index; }

void SendBufBlock::Retrieve(int len) {
  m_index += len;
  // LOG_DEBUG("index  %d , size: %d  len %d\n", m_index, m_nLen, m_index);
  assert(m_index <= m_nLen);
}

bool SendBufBlock::Finish() { return m_index == m_nLen; }

void SendBufBlock::Append(const char *data, int len) {
  if (m_capcity > m_nLen + len) {
    memcpy(m_pBuf + m_nLen, data, len);
    m_nLen += len;
  } else {
    int new_len = std::max(2 * m_capcity, m_nLen + len);
    m_pBuf = (char *)realloc(m_pBuf, new_len);
    CHECK(m_pBuf);
    memcpy(m_pBuf + m_nLen, data, len);
    m_nLen = new_len;
  }
}

void SendBufBlock::Clear() {
  SafeFree(m_pBuf);
  m_index = 0;
  m_capcity = 0;
  m_nLen = 0;
}

std::string SendBufBlock::ToString() {
  return std::string(m_pBuf + m_index, WaitSendLen());
}

const char *SendBufBlock::Header() {
  // LOG_DEBUG("index  %d , size: %d\n", m_index, m_nLen);
  return m_pBuf + m_index;
}

const char *SendBufBlock::Data() { return m_pBuf; }

int SendBufBlock::Len() { return m_nLen; }

WriteSendBufList::WriteSendBufList() {}

WriteSendBufList::~WriteSendBufList() {}

void WriteSendBufList::Append(const char *pdata, int len) {
  m_buf.emplace_back(std::make_shared<SendBufBlock>(pdata, len));
}

void WriteSendBufList::Append(const SendBufBlock::Ptr &blk) {
  m_buf.emplace_back(blk);
}

int WriteSendBufList::Send(int sockfd) {
  int ret;
  do {
    if (m_buf.empty()) {
      return 0;
    }

    auto &blk = m_buf.front();
    // LOG(INFO) << "send:" << blk->ToString();
    ret = ::send(sockfd, blk->Header(), blk->WaitSendLen(), 0);
    // LOG(INFO) << "send res " << ret;
    if (ret > 0) {
      blk->Retrieve(ret);
      if (blk->Finish()) {
        m_buf.pop_front();
      }
    } else if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN) {
        ret = 0;
      } else {
        ret = -1;
      }
    } else {
      ret = -1;
    }
  } while (ret > 0);

  return ret;
}

int WriteSendBufList::QueueSize() { return m_buf.size(); }
