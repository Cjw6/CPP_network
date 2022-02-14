#include "media/ReadAACFile.h"
#include "util/Adler32.h"
#include "util/ByteBuffer.h"
#include "util/File.h"

#include "gtest/gtest.h"

TEST(ReadAACFile, 1) {
  ReadAACFile r;
  std::string src_path = "resource/test.aac";
  std::string dst_path = "resource/testReadAAC.aac";
  ByteBuffer buf;

  char *aac_data_src = nullptr;
  int aac_size_src = ReadAllFileData(src_path.c_str(), aac_data_src);
  EXPECT_TRUE(aac_size_src > 0);

  unsigned int check_src = Adler32((unsigned char *)aac_data_src, aac_size_src);

  FILE *write_aac_fp = fopen(dst_path.c_str(), "wb+");
  EXPECT_TRUE(write_aac_fp);

  r.Open(src_path);
  while (1) {
    if (r.ReadFrame(buf, 3 * 1024) < 0) {
      break;
    }
    //
    fwrite(buf.Begin(), 1, buf.Size(), write_aac_fp);
  }

  fclose(write_aac_fp);
  write_aac_fp = nullptr;

  char *aac_data_dst = nullptr;
  int h264_size_dst = ReadAllFileData(dst_path.c_str(), aac_data_dst);

  EXPECT_TRUE(h264_size_dst);
  EXPECT_EQ(aac_size_src, h264_size_dst);

  unsigned int check_dst =
      Adler32((unsigned char *)aac_data_dst, h264_size_dst);

  EXPECT_EQ(check_src, check_dst);
  printf("samplerate %u \n,channel cfg %u\n",
         r.GetAdtsHeader().samplingFreqIndex, r.GetAdtsHeader().channelCfg);
  free(aac_data_dst);
  free(aac_data_src);
}