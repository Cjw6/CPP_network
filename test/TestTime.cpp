#include "gtest/gtest.h"

#include "util/Timer.h"

TEST(TimeUs, 1) {
  TimeUs t1 = TimeUs::Now();
  TimeUs t3 = t1;
  TimeUs t2 = TimeUs::Now();

  EXPECT_TRUE(t1 == t3);
  EXPECT_TRUE(t1 < t2);
  EXPECT_TRUE(t2 > t1);
  t3.AddTime(100);
  EXPECT_EQ(t1.GetAllUs() + 100, t3.GetAllUs());
}