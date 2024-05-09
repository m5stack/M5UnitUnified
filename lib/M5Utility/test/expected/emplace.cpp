#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>
#include <memory>
#include <vector>
#include <tuple>
namespace {
struct takes_init_and_variadic {
  std::vector<int> v;
  std::tuple<int, int> t;
  template <class... Args>
  takes_init_and_variadic(std::initializer_list<int> l, Args &&... args)
      : v(l), t(std::forward<Args>(args)...) {}
};
}

TEST(Expected, Emplace) {
    {
        m5::stl::expected<std::unique_ptr<int>,int> e;
        e.emplace(new int{42});
        EXPECT_TRUE(e);
        EXPECT_TRUE(**e == 42);
    }

    {
        m5::stl::expected<std::vector<int>,int> e;
        e.emplace({0,1});
        EXPECT_TRUE(e);
        EXPECT_TRUE((*e)[0] == 0);
        EXPECT_TRUE((*e)[1] == 1);
    }

    {
        m5::stl::expected<std::tuple<int,int>,int> e;
        e.emplace(2,3);
        EXPECT_TRUE(e);
        EXPECT_TRUE(std::get<0>(*e) == 2);
        EXPECT_TRUE(std::get<1>(*e) == 3);
    }

    {
        m5::stl::expected<::takes_init_and_variadic,int> e  = m5::stl::make_unexpected(0);
        e.emplace({0,1}, 2, 3);
        EXPECT_TRUE(e);
        EXPECT_TRUE(e->v[0] == 0);
        EXPECT_TRUE(e->v[1] == 1);
        EXPECT_TRUE(std::get<0>(e->t) == 2);
        EXPECT_TRUE(std::get<1>(e->t) == 3);
    }
}
