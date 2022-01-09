#include "MsgBuf.h"
#include "EncodeInt.h"

#include <list>

MsgBuf::MsgBuf(const char* data, int len)
{
	this->insert(end(), data, data + len);
}

void MsgBuf::Append(const char* data, int len)
{
	this->insert(end(), data, data + len);
}

void MsgBuf::PutFixedUint8(uint8_t value)
{
	Append(reinterpret_cast<char*>(&value), 1);
}

void MsgBuf::PutFixedUint32(uint32_t value)
{
	char buf[sizeof(value)];
	EncodeFixed32(buf, value);
	Append(buf, sizeof(buf));
}

void MsgBuf::PutFixedUint64(uint64_t value)
{
	char buf[sizeof(value)];
	EncodeFixed64(buf, value);
	Append(buf, sizeof(buf));
}

using MsgBufList=std::list<MsgBuf>;


