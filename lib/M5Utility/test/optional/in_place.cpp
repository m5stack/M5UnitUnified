#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

#include <tuple>
#include <vector>

namespace in_place {
struct takes_init_and_variadic {
  std::vector<int> v;
  std::tuple<int, int> t;
  template <class... Args>
  takes_init_and_variadic(std::initializer_list<int> l, Args &&... args)
      : v(l), t(std::forward<Args>(args)...) {}
};
}

TEST(Optional, Inplace) {
  m5::stl::optional<int> o1{m5::stl::in_place};
  m5::stl::optional<int> o2(m5::stl::in_place);
  EXPECT_TRUE(o1);
  EXPECT_TRUE(o1 == 0);
  EXPECT_TRUE(o2);
  EXPECT_TRUE(o2 == 0);

  m5::stl::optional<int> o3(m5::stl::in_place, 42);
  EXPECT_TRUE(o3 == 42);

  m5::stl::optional<std::tuple<int, int>> o4(m5::stl::in_place, 0, 1);
  EXPECT_TRUE(o4);
  EXPECT_TRUE(std::get<0>(*o4) == 0);
  EXPECT_TRUE(std::get<1>(*o4) == 1);

  m5::stl::optional<std::vector<int>> o5(m5::stl::in_place, {0, 1});
  EXPECT_TRUE(o5);
  EXPECT_TRUE((*o5)[0] == 0);
  EXPECT_TRUE((*o5)[1] == 1);

  m5::stl::optional<in_place::takes_init_and_variadic> o6(m5::stl::in_place, {0, 1}, 2, 3);
  EXPECT_TRUE(o6->v[0] == 0);
  EXPECT_TRUE(o6->v[1] == 1);
  EXPECT_TRUE(std::get<0>(o6->t) == 2);
  EXPECT_TRUE(std::get<1>(o6->t) == 3);
}
