#include "gtest/gtest.h"

#include "util/TimerUs.h"

TEST(TimeUs, 1) {
  TimeUs t1 = TimeUs::Now();
  TimeUs t3 = t1;
  TimeUs t2 = TimeUs::Now();

  EXPECT_TRUE(t1 == t3);
  EXPECT_TRUE(t1 < t2);
  EXPECT_TRUE(t2 > t1);
  t3.AddTimeUs(100);
  EXPECT_EQ(t1.GetTimeUs() + 100, t3.GetTimeUs());
}