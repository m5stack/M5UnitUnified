#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>
#include "helper.hpp"

TEST(Optional, Constexpr) {
#if !defined(TL_OPTIONAL_MSVC2015) && defined(TL_OPTIONAL_CXX14)
  {
    SCOPED_TRACE("empty construct");
    constexpr m5::stl::optional<int> o2{};
    constexpr m5::stl::optional<int> o3 = {};
    constexpr m5::stl::optional<int> o4 = m5::stl::nullopt;
    constexpr m5::stl::optional<int> o5 = {m5::stl::nullopt};
    constexpr m5::stl::optional<int> o6(m5::stl::nullopt);

    STATIC_EXPECT_TRUE(!o2);
    STATIC_EXPECT_TRUE(!o3);
    STATIC_EXPECT_TRUE(!o4);
    STATIC_EXPECT_TRUE(!o5);
    STATIC_EXPECT_TRUE(!o6);
  }
  {
    SCOPED_TRACE("value construct");
    constexpr m5::stl::optional<int> o1 = 42;
    constexpr m5::stl::optional<int> o2{42};
    constexpr m5::stl::optional<int> o3(42);
    constexpr m5::stl::optional<int> o4 = {42};
    constexpr int i = 42;
    constexpr m5::stl::optional<int> o5 = std::move(i);
    constexpr m5::stl::optional<int> o6{std::move(i)};
    constexpr m5::stl::optional<int> o7(std::move(i));
    constexpr m5::stl::optional<int> o8 = {std::move(i)};

    STATIC_EXPECT_TRUE(*o1 == 42);
    STATIC_EXPECT_TRUE(*o2 == 42);
    STATIC_EXPECT_TRUE(*o3 == 42);
    STATIC_EXPECT_TRUE(*o4 == 42);
    STATIC_EXPECT_TRUE(*o5 == 42);
    STATIC_EXPECT_TRUE(*o6 == 42);
    STATIC_EXPECT_TRUE(*o7 == 42);
    STATIC_EXPECT_TRUE(*o8 == 42);
  }
  #endif
}
