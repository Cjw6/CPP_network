#include "media/ReadH264File.h"
#include "util/Log.h"

ReadH264File::ReadH264File() : fp_(nullptr) {}

ReadH264File::~ReadH264File() {
  if (fp_) {
    fclose(fp_);
  }
}

int ReadH264File::Open(std::string &path) {
  fp_ = fopen(path.c_str(), "rb");
  if (!fp_) {
    LOG(ERROR) << "open file:" << path << " fail";
    return -1;
  }
  return 1;
}

int ReadH264File::startCode3(char *buf) {
  if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
    return 1;
  else
    return 0;
}

int ReadH264File::startCode4(char *buf) {
  if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
    return 1;
  else
    return 0;
}

int ReadH264File::ReadFrame(ByteBuffer &piece, int size) {
  if (!fp_) {
    return -1;
  }

  piece.RetrieveAll();
  piece.Resize(size);
  if (feof(fp_)) {
    fclose(fp_);
    fp_ = nullptr;
    return -1;
  }

  int n = fread(piece.Begin(), 1, piece.Capcity(), fp_);
  if (n <= 0) {
    fclose(fp_);
    fp_ = nullptr;
    return -1;
  }

  // LOG(INFO) << "read n:" << n;

  char *data = piece.Begin();
  int start_code = 0;
  if (startCode3(data)) {
    start_code = 3;
  } else if (startCode4(data)) {
    start_code = 4;
  } else {
    return -1;
  }

  piece.SetReadIndex(start_code);

  data += start_code;
  char *header = data;

  while (header - piece.Begin() < n - 2) {
    // LOG(INFO) << header[0] - '\0' << " " << header[1] - '\0' << " "
    //           << header[2] - '\0' << " " << header[3] - '\0';
    if (startCode3(header) && start_code == 3) {
      piece.SetWriteIndex(header - piece.Begin());
      fseek(fp_, header - piece.Begin() - n, SEEK_CUR);
      // LOG(INFO) << header - piece.Begin();
      return 1;
    } else if (startCode4(header) && start_code == 4) {
      piece.SetWriteIndex(header - piece.Begin());
      fseek(fp_, header - piece.Begin() - n, SEEK_CUR);
      // LOG(INFO) << header - piece.Begin();
      return 1;
    } else {
      header++;
    }
  }
  piece.SetWriteIndex(n);
  return 1;
}