#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

struct move_detector {
  move_detector() = default;
  move_detector(move_detector &&rhs) { rhs.been_moved = true; }
  bool been_moved = false;
};

TEST(Optional, Observers) {
  m5::stl::optional<int> o1 = 42;
  m5::stl::optional<int> o2;
  const m5::stl::optional<int> o3 = 42;

  EXPECT_TRUE(*o1 == 42);
  EXPECT_TRUE(*o1 == o1.value());
  EXPECT_TRUE(o2.value_or(42) == 42);
  EXPECT_TRUE(o3.value() == 42);
  auto success = std::is_same<decltype(o1.value()), int &>::value;
  EXPECT_TRUE(success);
  success = std::is_same<decltype(o3.value()), const int &>::value;
  EXPECT_TRUE(success);
  success = std::is_same<decltype(std::move(o1).value()), int &&>::value;
  EXPECT_TRUE(success);

  #ifndef TL_OPTIONAL_NO_CONSTRR
  success = std::is_same<decltype(std::move(o3).value()), const int &&>::value;
  EXPECT_TRUE(success);
  #endif

  m5::stl::optional<move_detector> o4{m5::stl::in_place};
  move_detector o5 = std::move(o4).value();
  EXPECT_TRUE(o4->been_moved);
  EXPECT_TRUE(!o5.been_moved);
}
