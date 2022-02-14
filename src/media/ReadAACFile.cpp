#include "ReadAACFile.h"
#include "util/ByteBuffer.h"
#include "util/Log.h"

ReadAACFile::ReadAACFile() : fp_(nullptr) {}

ReadAACFile::~ReadAACFile() {
  if (fp_) {
    fclose(fp_);
  }
}

int ReadAACFile::Open(std::string &path) {
  fp_ = fopen(path.c_str(), "rb");
  if (!fp_) {
    LOG(ERROR) << "open file:" << path << " fail";
    return -1;
  }
  return 1;
};

int ReadAACFile::ReadFrame(ByteBuffer &piece, int size) {
  if (!fp_) {
    LOG(ERROR) << "fp_ is null";
    return -1;
  }

  piece.RetrieveAll();
  piece.Resize(size);

  if (feof(fp_)) {
    LOG(WARNING) << "read  acc file tail";
    fclose(fp_);
    fp_ = nullptr;
    return -1;
  }

  int n = fread(piece.WriteBegin(), 1, 7, fp_);
  if (n != 7) {
    LOG(ERROR) << "read 7 aac header error";
    return -1;
  }

  // AdtsHeader adts_;
  if (parseAdtsHeader(reinterpret_cast<uint8_t *>(piece.WriteBegin()), &adts_)) {
    LOG(ERROR) << "parseAdtsHeader error";
    return -1;
  }
  piece.SetWriteIndex(7);

  auto &len = adts_.aacFrameLength;
  n = fread(piece.WriteBegin(), 1, len - 7, fp_);
  if (n != len - 7) {
    LOG(ERROR) << "read acc data error";
    return -1;
  }
  piece.SetWriteIndex(len);
  return 1;
}

int ReadAACFile::parseAdtsHeader(uint8_t *in, struct AdtsHeader *res) {
  static int frame_number = 0;
  memset(res, 0, sizeof(*res));

  if ((in[0] == 0xFF) && ((in[1] & 0xF0) == 0xF0)) {
    res->id = ((unsigned int)in[1] & 0x08) >> 3;
    res->layer = ((unsigned int)in[1] & 0x06) >> 1;
    res->protectionAbsent = (unsigned int)in[1] & 0x01;
    res->profile = ((unsigned int)in[2] & 0xc0) >> 6;
    res->samplingFreqIndex = ((unsigned int)in[2] & 0x3c) >> 2;
    res->privateBit = ((unsigned int)in[2] & 0x02) >> 1;
    res->channelCfg = ((((unsigned int)in[2] & 0x01) << 2) |
                       (((unsigned int)in[3] & 0xc0) >> 6));
    res->originalCopy = ((unsigned int)in[3] & 0x20) >> 5;
    res->home = ((unsigned int)in[3] & 0x10) >> 4;
    res->copyrightIdentificationBit = ((unsigned int)in[3] & 0x08) >> 3;
    res->copyrightIdentificationStart = (unsigned int)in[3] & 0x04 >> 2;
    res->aacFrameLength = (((((unsigned int)in[3]) & 0x03) << 11) |
                           (((unsigned int)in[4] & 0xFF) << 3) |
                           ((unsigned int)in[5] & 0xE0) >> 5);
    res->adtsBufferFullness =
        (((unsigned int)in[5] & 0x1f) << 6 | ((unsigned int)in[6] & 0xfc) >> 2);
    res->numberOfRawDataBlockInFrame = ((unsigned int)in[6] & 0x03);

    return 0;
  } else {
    printf("failed to parse adts header\n");
    return -1;
  }
}
