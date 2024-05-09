#include <gtest/gtest.h>
//#include <stdexcept>

//#define TL_ASSERT(cond) if (!(cond)) { throw std::runtime_error(std::string("assertion failure")); }

#include <m5_utility/stl/expected.hpp>
#if 0
TEST(Expected, Assertions) {
  m5::stl::expected<int,int> o1 = 42;
  EXPECT_ANY_THROW(o1.error());

  m5::stl::expected<int,int> o2 {m5::stl::unexpect, 0};
  EXPECT_ANY_THROW(*o2);

  struct foo { int bar; };
  m5::stl::expected<struct foo,int> o3 {m5::stl::unexpect, 0};
  EXPECT_ANY_THROW(o3->bar);
}
#endif
