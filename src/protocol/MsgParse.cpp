#include "util/Adler32.h"
#include "MsgParse.h"
#include "MsgCodec.h"
#include <cassert>
#include "EncodeInt.h"


MsgUnitValue::MsgUnitValue():type(CODEC_NULL){}

MsgUnitValue::MsgUnitValue(MsgCodecType _type,MsgBufSlice _v):type(_type),v(_v){}

MsgParse::MsgParse(MsgBufSlice msg_buf) : buf_(msg_buf), elem_num_(0), data_len_(0), parse_index_(0)
{
	assert(msg_buf.data);
	assert(msg_buf.size > 0);
}

int32_t MsgParse::ParseHeader(uint32_t& cmd1, uint32_t& cmd2)
{
	data_len_ = DecodeFixed32(buf_.data + sizeof(MsgCodec::started_code));
	elem_num_ = DecodeFixed32(buf_.data + sizeof(MsgCodec::started_code) + sizeof(uint32_t));
	//printf("%s %d %d \n", __func__, data_len_, elem_num_);

	if (data_len_ + sizeof(uint32_t) != buf_.size)
	{
		return -1;
	}

	uint32_t adler32_cal = Adler32((unsigned char*)buf_.data, data_len_);
	uint32_t adler32_src = DecodeFixed32(buf_.data + data_len_);

	if (adler32_cal != adler32_src)
	{
		printf("adler32 error\n");
		return -1;
	}

	cmd1 = DecodeFixed32(buf_.data + 2 * sizeof(uint32_t)+sizeof(MsgCodec::started_code));
	cmd2 = DecodeFixed32(buf_.data + 3 * sizeof(uint32_t)+sizeof(MsgCodec::started_code));
	parse_index_ = MsgCodec::reserve_len;
	return 1;
}

bool MsgParse::ParseAllUnit()
{
	MsgBufSlice dst_key;
  MsgBufSlice dst_value;
  MsgCodecType dst_type;
	while (!ParseEnd()) {
		ParseUnit(dst_key, dst_value, dst_type);
		// MsgUnitValue v(dst_type,dst_value);
		// msg_map_[dst_key.ToString()]=v;
	}
	return true;
}

bool MsgParse::ParseEnd()
{
	//LOG_DEBUG("parse end?  %u %u \n", parse_index_, data_len_);
	return parse_index_ >= data_len_;
}

bool MsgParse::CheckAdler32(MsgBufSlice& msg_buf)
{
	const char* data = msg_buf.data;
	uint32_t adler32_cal = Adler32((unsigned char*)data,msg_buf.size-sizeof(uint32_t));
	uint32_t adler32_src = DecodeFixed32(data + msg_buf.size-sizeof(uint32_t));
	return adler32_cal == adler32_src;
}

bool MsgParse::ParseUnit(MsgBufSlice& key, MsgBufSlice& value, MsgCodecType& type)
{
	type = (MsgCodecType) * reinterpret_cast<uint8_t*>(BufBegin());
	Retrieve(sizeof(uint8_t));

	uint32_t key_len = DecodeFixed32(BufBegin());
	Retrieve(sizeof(uint32_t));

	key = MsgBufSlice(BufBegin(), key_len);
	Retrieve(key_len);

	//printf("%d %d %d %d\n", *BufBegin(), *(BufBegin() + 1), *(BufBegin() + 2), *(BufBegin() + 3));
	uint32_t value_len = DecodeFixed32(BufBegin());
	Retrieve(sizeof(uint32_t));
	printf("key_len %u,value len %u\n", key_len, value_len);

	value = MsgBufSlice(BufBegin(), value_len);
	Retrieve(value_len);

	//printf("buf_.data%d  parse_index_%d\n", buf_.size, parse_index_);
	return true;
}

char* MsgParse::BufBegin()
{
	return const_cast<char*>(buf_.data + parse_index_);
}

void MsgParse::Retrieve(uint32_t len)
{
	parse_index_ += len;
	//printf("paese index  %d \n", parse_index_);
}

void MsgParse::ParseArray(MsgBufSlice& tag, MsgBufSliceArray& msgSlices)\
{
	uint32_t elem_num = DecodeFixed32(tag.data);
	uint32_t parse_index = sizeof(uint32_t);

	for (uint32_t i = 0; i < elem_num; i++)
	{
		uint32_t elem_len = DecodeFixed32(tag.data + parse_index);
		parse_index += sizeof(uint32_t);

		MsgBufSlice dst_buf(tag.data + parse_index, elem_len);
		parse_index += elem_len;

		msgSlices.push_back(dst_buf);
	}
}

const char* MsgParse::FindMsgHeader(MsgBufSlice& msgSlice)
{
	const char* pDataHeader = msgSlice.data;
	const char* pStartCode = MsgCodec::started_code;
	uint32_t& data_len = msgSlice.size;

	if (data_len < sizeof(MsgCodec::started_code))
		return NULL;

	for (uint32_t i = 0; i < data_len-sizeof(MsgCodec::started_code); i++)
	{
		bool bFound = true;
		for (uint32_t j=0; j < sizeof(MsgCodec::started_code); j++)
		{
			if (*(pDataHeader+j) != MsgCodec::started_code[j])
			{
				bFound = false;
				break;
			}
		}
		if(bFound)
			return pDataHeader;
		pDataHeader++;
	}
	return NULL;
}

int MsgParse::SplitBufToMsgBufBlockList(MsgBufSlice bufSlice, MsgBufList& listBufBlks)
{
	uint32_t nParseIndex = 0;
	uint32_t nWaitParse = bufSlice.size;
	MsgBufSlice cur_bufSlice = bufSlice;

	while (1)
	{
		const char* pDataHeader = MsgParse::FindMsgHeader(cur_bufSlice);
		if (pDataHeader == NULL)
		{
			printf("return %d\n", __LINE__);
			return bufSlice.size;
		}
		else
		{
			int n=pDataHeader-cur_bufSlice.data;
			nParseIndex += n;
			nWaitParse -= n;
		}

		uint32_t distance = pDataHeader - bufSlice.data;
		uint32_t nDataLen = DecodeFixed32(pDataHeader + sizeof(MsgCodec::started_code));
		
		printf("%d %d %d\n", distance, nDataLen, bufSlice.size);
		if (distance + nDataLen+sizeof(uint32_t) > bufSlice.size)
		{
			printf("return %d\n", __LINE__);
			return nParseIndex;
		}

		MsgBufSlice slice(pDataHeader, nDataLen + sizeof(uint32_t));
		
		if (!MsgParse::CheckAdler32(slice))
		{
			printf("%d %ld \n", nWaitParse, sizeof(MsgCodec::started_code));
			if (nWaitParse < sizeof(MsgCodec::started_code))
			{
				printf("return %d \n", __LINE__);
				break;
			}
				
			pDataHeader += sizeof(MsgCodec::started_code);
			nParseIndex += sizeof(MsgCodec::started_code);
			nWaitParse -= sizeof(MsgCodec::started_code);
		}
		else
		{	//BufFixedBlock(const char* data, int len);
			listBufBlks.emplace_back(pDataHeader,(int)(nDataLen + sizeof(uint32_t)));
			nParseIndex += (nDataLen + sizeof(MsgCodec::started_code));
			nWaitParse -= (nDataLen + sizeof(MsgCodec::started_code));
			pDataHeader += (nDataLen + sizeof(MsgCodec::started_code));
		}
		
		if (nWaitParse <= sizeof(MsgCodec::reserve_len))
		{
			printf("return %d\n", __LINE__);
			break;
		}
			

		cur_bufSlice.data = pDataHeader;
		cur_bufSlice.size = nWaitParse;
	}
	printf("return %d\n", __LINE__);
	return nParseIndex;
}
