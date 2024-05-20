/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef TEST_HEKPER_HPP
#define TEST_HEKPER_HPP

// catch2 STATIC_REQUIRE
#define STATIC_EXPECT_TRUE(constexpr_cond)                     \
    do {                                                       \
        static_assert((constexpr_cond),                        \
                      "Occurrence of compile-time assertion"); \
    } while (0)

#endif
