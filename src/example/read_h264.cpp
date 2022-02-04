#include "media/Rtp.h"
#include <glog/logging.h>

int main() {
  ReadH264File r;
  std::string path = "resource/test.h264";
  r.Open(path);
  ByteBuffer buffer;
  int index = 0;

  while (1) {
    if (r.ReadFrame(buffer) < 0) {
      break;
    }
    LOG(INFO) << "frame cnt" << index++
              << " data len:" << buffer.ReadableSize();
  }
  return 0;
}