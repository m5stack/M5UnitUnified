/*!
  UnitTest for M5Utility

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
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
