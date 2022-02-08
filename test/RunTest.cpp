#include "gtest/gtest.h"

int main(int argc, char **argv) {
  chdir("/media/cjw/work1/ubuntu_project/new_project");
  char str[1024] = {0};
  getcwd(str, sizeof(str));
  printf("workpath %s\n", str);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}