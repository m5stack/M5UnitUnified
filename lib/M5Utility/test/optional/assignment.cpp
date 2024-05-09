#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

TEST(Optional, Assignment) {
    m5::stl::optional<int> o1 = 42;
    m5::stl::optional<int> o2 = 12;
    m5::stl::optional<int> o3;

    o1 = o1;
    EXPECT_TRUE(*o1 == 42);

    o1 = o2;
    EXPECT_TRUE(*o1 == 12);

    o1 = o3;
    EXPECT_TRUE(!o1);

    o1 = 42;
    EXPECT_TRUE(*o1 == 42);

    o1 = m5::stl::nullopt;
    EXPECT_TRUE(!o1);

    o1 = std::move(o2);
    EXPECT_TRUE(*o1 == 12);

    m5::stl::optional<short> o4 = 42;

    o1 = o4;
    EXPECT_TRUE(*o1 == 42);

    o1 = std::move(o4);
    EXPECT_TRUE(*o1 == 42);
}


TEST(Optional, AssignmentReference) {
    auto i = 42;
    auto j = 12;
    
    m5::stl::optional<int&> o1 = i;
    m5::stl::optional<int&> o2 = j;
    m5::stl::optional<int&> o3;

    o1 = o1;
    EXPECT_TRUE(*o1 == 42);
    EXPECT_TRUE(&*o1 == &i);    

    o1 = o2;
    EXPECT_TRUE(*o1 == 12);

    o1 = o3;
    EXPECT_TRUE(!o1);

    auto k = 42;
    o1 = k;
    EXPECT_TRUE(*o1 == 42);
    EXPECT_TRUE(*o1 == i);
    EXPECT_TRUE(*o1 == k);        
    EXPECT_TRUE(&*o1 != &i);
    EXPECT_TRUE(&*o1 == &k);

    k = 12;
    EXPECT_TRUE(*o1 == 12);

    o1 = m5::stl::nullopt;
    EXPECT_TRUE(!o1);

    o1 = std::move(o2);
    EXPECT_TRUE(*o1 == 12);
}
