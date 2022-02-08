#include "MsgCodec.h"
#include "MsgBuf.h"
#include "MsgBufSlice.h"
#include "EncodeInt.h"
#include "util/Adler32.h"
#include <cassert>

uint32_t MsgCodec::reserve_len = 4 * sizeof(uint32_t) + sizeof(MsgCodec::started_code); //保存 总子节的长度  保存元素的个数 添加主类型 子类型  
char MsgCodec::started_code[4] = { 1,1,1,0 };

MsgCodec::MsgCodec(MsgBuf* buf_blk) : buf_(buf_blk), elem_num_(0), data_len_(0)
{
	assert(buf_blk);
}

MsgCodec::~MsgCodec()
{
}

void MsgCodec::StartGen(uint32_t cmd1, uint32_t cmd2)
{
	elem_num_ = 0;
	data_len_ = reserve_len;

	buf_->clear();
	buf_->resize(reserve_len, 0);
	std::copy(std::begin(started_code), std::end(started_code), buf_->data());
	EncodeFixed32(buf_->data() + 2 * sizeof(uint32_t)+sizeof(MsgCodec::started_code), cmd1);
	EncodeFixed32(buf_->data() + 3 * sizeof(uint32_t)+sizeof(MsgCodec::started_code), cmd2);

}
void MsgCodec::FinshGen()
{
	EncodeFixed32(buf_->data() + sizeof(started_code), data_len_);
	EncodeFixed32(buf_->data() + sizeof(started_code) + sizeof(uint32_t), elem_num_);

	unsigned int adler32 = Adler32((unsigned char*)buf_->data(), static_cast<unsigned int>(buf_->size()));
	buf_->PutFixedUint32(adler32);
	//printf("buf_ize:%d data_len:%d adler32:%u\n", static_cast<int>(buf_->size()), data_len_, adler32);
}

void MsgCodec::PutKVUInt32(MsgBufSlice k, uint32_t v)
{
	buf_->PutFixedUint8(CODEC_KV_UINT32);	 //type
	buf_->PutFixedUint32(k.size);	 //k len
	buf_->Append(k.data, k.size);	 //k data
	buf_->PutFixedUint32(sizeof(v)); //v len
	buf_->PutFixedUint32(v);		 //v data

	elem_num_++;
	uint32_t all_size = sizeof(uint8_t) + sizeof(uint32_t) * 3 + k.size;
	data_len_ += all_size;
}

void MsgCodec::PutKVString(MsgBufSlice k, MsgBufSlice v)
{
	buf_->PutFixedUint8(CODEC_KV_STRING); //type
	buf_->PutFixedUint32(k.size);	//k len
	buf_->Append(k.data, k.size);	//k data
	buf_->PutFixedUint32(v.size);	//v len
	buf_->Append(v.data, v.size);	//v data

	elem_num_++;
	uint32_t all_size = sizeof(uint8_t) + sizeof(uint32_t) * 2 + k.size + v.size;
	data_len_ += all_size;
}

void MsgCodec::PutArray(MsgBufSlice k, std::vector<MsgBufSlice>& buf_blks, MsgCodecType type)
{
	buf_->PutFixedUint8(type);

	buf_->PutFixedUint32(k.size); //k len
	buf_->Append(k.data, k.size); //k data

	//buf_->back()
	buf_->PutFixedUint32(0);
	int pos_array_offset = buf_->end() - buf_->begin() - 4;								//&buf_->back() - sizeof(uint32_t) + 1;
	uint32_t all_size = sizeof(uint8_t) + sizeof(uint32_t) + k.size + sizeof(uint32_t); //tag k

	buf_->PutFixedUint32(static_cast<uint32_t>(buf_blks.size())); //array_num,
	elem_num_++;

	//警告： 由于parse 时计算数组长度后 retr
	uint32_t array_size = 1 * sizeof(uint32_t); //array_elem_num  array_

	for (uint8_t i = 0; i < buf_blks.size(); i++)
		// for (auto& blk : buf_blks)
	{
		MsgBufSlice& blk = buf_blks[i];
		buf_->PutFixedUint32(blk.size);	  //v len
		buf_->Append(blk.data, blk.size); //v data
		array_size += (sizeof(uint32_t) + blk.size);
	}
	all_size += array_size;
	data_len_ += all_size;
	EncodeFixed32(buf_->data() + pos_array_offset, array_size);
	//printf("all size %u %u\n", all_size, array_size);
}