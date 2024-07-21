#include "example/add.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(addTest, addZeroTest) { EXPECT_EQ(add(0, 0), 0); }

TEST(addTest, addPositiveTest) { EXPECT_EQ(add(1, 1), 2); }

int main(int argc, char *argv[]) {
  // 初始化 Google Test
  ::testing::InitGoogleTest(&argc, argv);
  // 运行所有测试案例
  return RUN_ALL_TESTS();
}