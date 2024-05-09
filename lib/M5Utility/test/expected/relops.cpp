#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

TEST(Expected, RelationalOperators) {
  m5::stl::expected<int, int> o1 = 42;
  m5::stl::expected<int, int> o2{m5::stl::unexpect, 0};
  const m5::stl::expected<int, int> o3 = 42;

  EXPECT_TRUE(o1 == o1);
  EXPECT_TRUE(o1 != o2);
  EXPECT_TRUE(o1 == o3);
  EXPECT_TRUE(o3 == o3);

  m5::stl::expected<void, int> o6;

  EXPECT_TRUE(o6 == o6);
}
