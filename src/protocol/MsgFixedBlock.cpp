#include "MsgFixedBlock.h"
#include "MsgBuf.h"

#include <algorithm>
#include <cassert>
#include <cstring>

MsgFixedBlock::MsgFixedBlock(const char *data, int len)
    : m_pBuf(nullptr), m_nLen(0) {
  assert(data);
  assert(len > 0);
  m_pBuf = new char[len];
  memcpy(m_pBuf, data, m_nLen);
}

MsgFixedBlock::MsgFixedBlock(MsgBuf &bufBlk) {
  m_nLen = bufBlk.size();
  m_pBuf = new char[m_nLen];
  assert(m_pBuf);
  std::copy(bufBlk.begin(), bufBlk.end(), m_pBuf);
}

MsgFixedBlock::~MsgFixedBlock() {
  // LOG_DEBUG("destruct .....\n");
  if (m_pBuf) {
    delete[] m_pBuf;
  }
}

const char *MsgFixedBlock::Data() { return m_pBuf; }

int MsgFixedBlock::Len() { return m_nLen; }