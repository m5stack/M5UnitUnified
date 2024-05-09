#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include <vector>

namespace constructor {
struct foo {
  foo() = default;
  foo(foo&) = delete;
  foo(foo&&) {};
};
}

TEST(Optional, Constructors) {
  m5::stl::optional<int> o1;
  EXPECT_TRUE(!o1);

  m5::stl::optional<int> o2 = m5::stl::nullopt;
  EXPECT_TRUE(!o2);

  m5::stl::optional<int> o3 = 42;
  EXPECT_TRUE(*o3 == 42);

  m5::stl::optional<int> o4 = o3;
  EXPECT_TRUE(*o4 == 42);

  m5::stl::optional<int> o5 = o1;
  EXPECT_TRUE(!o5);

  m5::stl::optional<int> o6 = std::move(o3);
  EXPECT_TRUE(*o6 == 42);

  m5::stl::optional<short> o7 = 42;
  EXPECT_TRUE(*o7 == 42);

  m5::stl::optional<int> o8 = o7;
  EXPECT_TRUE(*o8 == 42);

  m5::stl::optional<int> o9 = std::move(o7);
  EXPECT_TRUE(*o9 == 42);

  {
    m5::stl::optional<int &> o;
    EXPECT_TRUE(!o);

    m5::stl::optional<int &> oo = o;
    EXPECT_TRUE(!oo);
  }

  {
    auto i = 42;
    m5::stl::optional<int &> o = i;
    EXPECT_TRUE(o);
    EXPECT_TRUE(*o == 42);

    m5::stl::optional<int &> oo = o;
    EXPECT_TRUE(oo);
    EXPECT_TRUE(*oo == 42);
  }

  std::vector<constructor::foo> v;
  v.emplace_back();
  m5::stl::optional<std::vector<constructor::foo>> ov = std::move(v);
  EXPECT_TRUE(ov->size() == 1);
}
