#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

TEST(Optional, Nullopt) {
  m5::stl::optional<int> o1 = m5::stl::nullopt;
  m5::stl::optional<int> o2{m5::stl::nullopt};
  m5::stl::optional<int> o3(m5::stl::nullopt);
  m5::stl::optional<int> o4 = {m5::stl::nullopt};

  EXPECT_TRUE(!o1);
  EXPECT_TRUE(!o2);
  EXPECT_TRUE(!o3);
  EXPECT_TRUE(!o4);

  EXPECT_TRUE(!std::is_default_constructible<m5::stl::nullopt_t>::value);
}
