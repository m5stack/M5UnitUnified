#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

TEST(Optional, SwapValue) {
  m5::stl::optional<int> o1 = 42;
  m5::stl::optional<int> o2 = 12;
  o1.swap(o2);
  EXPECT_TRUE(o1.value() == 12);
  EXPECT_TRUE(o2.value() == 42);
}

TEST(Optional, SwapValueWithNullIntialized) {
  m5::stl::optional<int> o1 = 42;
  m5::stl::optional<int> o2 = m5::stl::nullopt;
  o1.swap(o2);
  EXPECT_TRUE(!o1.has_value());
  EXPECT_TRUE(o2.value() == 42);
}

TEST(Optional,SwapNullIntializedWithValue) {
  m5::stl::optional<int> o1 = m5::stl::nullopt;
  m5::stl::optional<int> o2 = 42;
  o1.swap(o2);
  EXPECT_TRUE(o1.value() == 42);
  EXPECT_TRUE(!o2.has_value());
}
