#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include <utility>
#include <tuple>

TEST(Optional, Emplace) {
    m5::stl::optional<std::pair<std::pair<int,int>, std::pair<double, double>>> i;
    i.emplace(std::piecewise_construct, std::make_tuple(0,2), std::make_tuple(3,4));
    EXPECT_TRUE(i->first.first == 0);
    EXPECT_TRUE(i->first.second == 2);
    EXPECT_TRUE(i->second.first == 3);
    EXPECT_TRUE(i->second.second == 4);    
}

struct A {
    A() {
        throw std::exception();
    }
};

TEST(Optional, EmplaceWithException) {
    m5::stl::optional<A> a;
    EXPECT_ANY_THROW(a.emplace());
}
