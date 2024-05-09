#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include <tuple>
#include <vector>

namespace make_opt {
struct takes_init_and_variadic {
  std::vector<int> v;
  std::tuple<int, int> t;
  template <class... Args>
  takes_init_and_variadic(std::initializer_list<int> l, Args &&... args)
      : v(l), t(std::forward<Args>(args)...) {}
};
}

TEST(Optional, MakeOptional) {
  auto o1 = m5::stl::make_optional(42);
  auto o2 = m5::stl::optional<int>(42);

  constexpr bool is_same = std::is_same<decltype(o1), m5::stl::optional<int>>::value;
  EXPECT_TRUE(is_same);
  EXPECT_TRUE(o1 == o2);

  auto o3 = m5::stl::make_optional<std::tuple<int, int, int, int>>(0, 1, 2, 3);
  EXPECT_TRUE(std::get<0>(*o3) == 0);
  EXPECT_TRUE(std::get<1>(*o3) == 1);
  EXPECT_TRUE(std::get<2>(*o3) == 2);
  EXPECT_TRUE(std::get<3>(*o3) == 3);

  auto o4 = m5::stl::make_optional<std::vector<int>>({0, 1, 2, 3});
  EXPECT_TRUE(o4.value()[0] == 0);
  EXPECT_TRUE(o4.value()[1] == 1);
  EXPECT_TRUE(o4.value()[2] == 2);
  EXPECT_TRUE(o4.value()[3] == 3);

  auto o5 = m5::stl::make_optional<make_opt::takes_init_and_variadic>({0, 1}, 2, 3);
  EXPECT_TRUE(o5->v[0] == 0);
  EXPECT_TRUE(o5->v[1] == 1);
  EXPECT_TRUE(std::get<0>(o5->t) == 2);
  EXPECT_TRUE(std::get<1>(o5->t) == 3);

  auto i = 42;
  auto o6 = m5::stl::make_optional<int&>(i);
  EXPECT_TRUE((std::is_same<decltype(o6), m5::stl::optional<int&>>::value));
  EXPECT_TRUE(o6);
  EXPECT_TRUE(*o6 == 42);    
}
