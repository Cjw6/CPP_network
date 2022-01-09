#include "MsgBufSlice.h"

#include <cassert>
#include <cstring>

MsgBufSlice::MsgBufSlice() : data(NULL), size(0)
{
}

MsgBufSlice::MsgBufSlice(MsgBuf& k) : data(k.data()), size(k.size())
{
}

MsgBufSlice::MsgBufSlice(std::string& str) : data(str.data()), size(str.length() + 1)
{
}

MsgBufSlice::MsgBufSlice(const char* _data, uint32_t _len) : data(_data), size(_len)
{
	assert(data);
	assert(size > 0);
}

MsgBufSlice::MsgBufSlice(const char* _data) : data(_data)
{
	assert(_data);
	size = static_cast<uint32_t>(strlen(data) + 1);
	assert(size > 0);
}