#pragma once

#include <cstdint>

enum MsgCodecType : int32_t {
  CODEC_KV_UINT32 = 0,
  CODEC_KV_INT64 = 1,
  CODEC_KV_STRING = 2,
  CODEC_KV_BYTE = 3,
  CODEC_ARRAY_STRING = 4,
  CODEC_ARRAY_BYTE = 5,
  CODEC_NULL = 100
};
