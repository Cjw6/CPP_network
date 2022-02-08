#include "util/File.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

int FileSize(const char *filename) //获取文件名为filename的文件大小。
{
  FILE *fp = fopen(filename, "rb"); //打开文件。
  int size;
  if (fp == NULL) // 打开文件失败
    return -1;
  fseek(fp, 0, SEEK_END); //定位文件指针到文件尾。
  size = ftell(fp);       //获取文件指针偏移量，即文件大小。
  fclose(fp);             //关闭文件。
  return size;
}

int ReadAllFileData(const char *filename, char *&data) {
  assert(!data);

  int file_size = FileSize(filename);
  if (file_size < 0) {
    return file_size;
  }

  data = (char *)malloc(file_size);
  if (!data) {
    return -1;
  }

  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    free(data);
    data = nullptr;
    return -1;
  }

  int n = fread(data, 1, file_size, fp);
  if (n < 0) {
    free(data);
    data = nullptr;
    return -1;
  } else {
    return file_size;
  }
}
