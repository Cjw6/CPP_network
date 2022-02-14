#include <gtest/gtest.h>
#include <iostream>

#include "media/ReadH264File.h"
#include "util/Adler32.h"
#include "util/File.h"

TEST(ReadH264, 1) {
  ReadH264File r;

  std::string src_path = "resource/test.h264";
  std::string dst_path = "resource/testReadH264.h264";

  char *h264_data_src = nullptr;
  int h264_size_src = ReadAllFileData(src_path.c_str(), h264_data_src);
  EXPECT_TRUE(h264_size_src > 0);

  unsigned int check_src =
      Adler32((unsigned char *)h264_data_src, h264_size_src);

  r.Open(src_path);
  ByteBuffer buffer;
  int index = 0;

  FILE *write_h264_fp = fopen(dst_path.c_str(), "wb+");
  EXPECT_TRUE(write_h264_fp);

  while (1) {
    if (r.ReadFrame(buffer, 500 * 1024) < 0) {
      break;
    }
    // std::cout << "frame cnt" << index++
    //           << " data len:" << buffer.ReadableSize();

    fwrite(buffer.Begin(), 1, buffer.Size(), write_h264_fp);
  }
  if (buffer.Size()) {
    fwrite(buffer.Begin(), 1, buffer.Size(), write_h264_fp);
  }

  fclose(write_h264_fp);
  write_h264_fp=nullptr;

  char *h264_data_dst = nullptr;
  int h264_size_dst = ReadAllFileData(dst_path.c_str(), h264_data_dst);

  EXPECT_TRUE(h264_size_dst);
  EXPECT_EQ(h264_size_src, h264_size_dst);

  unsigned int check_dst =
      Adler32((unsigned char *)h264_data_dst, h264_size_dst);

  EXPECT_EQ(check_src, check_dst);

  free(h264_data_dst);
  free(h264_data_src);

  // return 0;
}