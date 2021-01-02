#include "gtest/gtest.h"
#include "lib/linlib.h"

TEST(HelloTest, GetGreet) {
  EXPECT_EQ(linlib::version_name(), "Liblib 0.1");
}

