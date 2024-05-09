#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>

struct move_detector {
  move_detector() = default;
  move_detector(move_detector &&rhs) { rhs.been_moved = true; }
  bool been_moved = false;
};

TEST(Expected, Observers) {
    m5::stl::expected<int,int> o1 = 42;
    m5::stl::expected<int,int> o2 {m5::stl::unexpect, 0};
    const m5::stl::expected<int,int> o3 = 42;

  EXPECT_TRUE(*o1 == 42);
  EXPECT_TRUE(*o1 == o1.value());
  EXPECT_TRUE(o2.value_or(42) == 42);
  EXPECT_TRUE(o2.error() == 0);
  EXPECT_TRUE(o3.value() == 42);
  auto success = std::is_same<decltype(o1.value()), int &>::value;
  EXPECT_TRUE(success);
  success = std::is_same<decltype(o3.value()), const int &>::value;
  EXPECT_TRUE(success);
  success = std::is_same<decltype(std::move(o1).value()), int &&>::value;
  EXPECT_TRUE(success);

  #ifndef TL_EXPECTED_NO_CONSTRR
  success = std::is_same<decltype(std::move(o3).value()), const int &&>::value;
  EXPECT_TRUE(success);
  #endif

  m5::stl::expected<move_detector,int> o4{m5::stl::in_place};
  move_detector o5 = std::move(o4).value();
  EXPECT_TRUE(o4->been_moved);
  EXPECT_TRUE(!o5.been_moved);

  // Add by GOB
  EXPECT_TRUE(o1.error_or(52) == 52);
  EXPECT_TRUE(o2.error_or(52) == 0);
}
