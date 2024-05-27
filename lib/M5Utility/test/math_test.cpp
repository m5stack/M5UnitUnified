/*
  UnitTest for M5Utility


  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

TEST(Utility, is_powerof2) {
    EXPECT_FALSE(m5::math::is_powerof2(0));
    EXPECT_TRUE(m5::math::is_powerof2(1));
    EXPECT_TRUE(m5::math::is_powerof2(2));
    EXPECT_FALSE(m5::math::is_powerof2(3));

    constexpr auto b = m5::math::is_powerof2(-1);
    EXPECT_FALSE(b);
    EXPECT_FALSE(m5::math::is_powerof2(-2));
}
