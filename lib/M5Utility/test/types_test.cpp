/*!
  UnitTest for M5Utility

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

using namespace m5::types;

TEST(Utility, Types_U16) {
    // Constructor
    constexpr big_uint16_t bg0;
    constexpr little_uint16_t lt0;
    EXPECT_EQ(bg0.u16, 0U);
    EXPECT_EQ(lt0.u16, 0U);

    big_uint16_t bg1{0x1234};
    little_uint16_t lt1{0x1234};
    EXPECT_EQ(bg1.u8[0], 0x12);
    EXPECT_EQ(bg1.u8[1], 0x34);
    EXPECT_EQ(lt1.u8[0], 0x34);
    EXPECT_EQ(lt1.u8[1], 0x12);

    constexpr big_uint16_t bg2{0x12, 0x34};
    constexpr little_uint16_t lt2{0x12, 0x34};
    EXPECT_EQ(bg2.u8[0], 0x12);
    EXPECT_EQ(bg2.u8[1], 0x34);
    EXPECT_EQ(lt2.u8[0], 0x12);
    EXPECT_EQ(lt2.u8[1], 0x34);

    big_uint16_t bg3    = bg1;
    little_uint16_t lt3 = lt1;
    EXPECT_EQ(bg3.u8[0], 0x12);
    EXPECT_EQ(bg3.u8[1], 0x34);
    EXPECT_EQ(lt3.u8[0], 0x34);
    EXPECT_EQ(lt3.u8[1], 0x12);

    big_uint16_t bg4    = std::move(bg3);
    little_uint16_t lt4 = std::move(lt3);
    EXPECT_EQ(bg4.u8[0], 0x12);
    EXPECT_EQ(bg4.u8[1], 0x34);
    EXPECT_EQ(lt4.u8[0], 0x34);
    EXPECT_EQ(lt4.u8[1], 0x12);

    // Assignment
    big_uint16_t bg5;
    little_uint16_t lt5;
    bg5 = bg4;
    lt5 = lt4;
    EXPECT_EQ(bg5.u8[0], 0x12);
    EXPECT_EQ(bg5.u8[1], 0x34);
    EXPECT_EQ(lt5.u8[0], 0x34);
    EXPECT_EQ(lt5.u8[1], 0x12);

    big_uint16_t bg60;
    big_uint16_t bg61;
    little_uint16_t lt60;
    little_uint16_t lt61;
    if (m5::endian::little) {
        bg60.operator= <false>(0x3412);  // big to big
        bg61.operator= <true>(0x1234);   // little to big
        lt60.operator= <false>(0x3412);  // big to little
        lt61.operator= <true>(0x1234);   // little to little
    } else {
        bg60.operator= <false>(0x1234);  // big to big
        bg61.operator= <true>(0x3412);   // little to big
        lt60.operator= <false>(0x1234);  // big to little
        lt61.operator= <true>(0x3412);   // little to little
    }
    EXPECT_EQ(bg60.u8[0], 0x12);
    EXPECT_EQ(bg60.u8[1], 0x34);
    EXPECT_EQ(bg61.u8[0], 0x12);
    EXPECT_EQ(bg61.u8[1], 0x34);
    EXPECT_EQ(lt60.u8[0], 0x34);
    EXPECT_EQ(lt60.u8[1], 0x12);
    EXPECT_EQ(lt61.u8[0], 0x34);
    EXPECT_EQ(lt61.u8[1], 0x12);

    big_uint16_t bg7;
    little_uint16_t lt7;
    bg7 = std::make_pair<int, int>(0x12, 0x34);
    lt7 = std::make_pair<int, int>(0x12, 0x34);
    EXPECT_EQ(bg7.u8[0], 0x12);
    EXPECT_EQ(bg7.u8[1], 0x34);
    EXPECT_EQ(lt7.u8[0], 0x12);
    EXPECT_EQ(lt7.u8[1], 0x34);

    // Compile error (static_assert)
    // bg7 = std::make_pair<float, double>( 1.2f, 3.4 );

    // Cast
    EXPECT_FALSE((bool)bg0);
    EXPECT_FALSE((bool)lt0);
    EXPECT_TRUE((bool)bg1);
    EXPECT_TRUE((bool)lt1);

    EXPECT_EQ((uint16_t)bg0, 00U);
    EXPECT_EQ((uint16_t)lt0, 00U);
    EXPECT_EQ((uint16_t)bg1, 0x1234);
    EXPECT_EQ((uint16_t)lt1, 0x1234);

    EXPECT_EQ(*(const uint8_t*)bg0, 0U);
    EXPECT_EQ(*(const uint8_t*)lt0, 0U);
    EXPECT_EQ(*(const uint8_t*)bg1, 0x12U);
    EXPECT_EQ(*(const uint8_t*)lt1, 0x34U);

    // set() is using in operator=
    // get() is using in cast to uint16
    // data() is using in cast to const uint8_t*
}

TEST(Utility, Types_U16_Compare) {
    big_uint16_t bg0{0x1234};
    little_uint16_t lt0{0x1234};

    EXPECT_EQ(bg0, lt0);
    EXPECT_GE(bg0, lt0);
    EXPECT_LE(bg0, lt0);
    EXPECT_FALSE(bg0 < lt0);
    EXPECT_FALSE(bg0 > lt0);

    {
        big_uint16_t bg1{0x1235};
        little_uint16_t lt1{0x1235};

        EXPECT_NE(bg0, bg1);
        EXPECT_NE(bg0, lt1);
        EXPECT_LT(bg0, bg1);
        EXPECT_LT(bg0, lt1);
        EXPECT_LE(bg0, bg1);
        EXPECT_LE(bg0, lt1);
        EXPECT_GE(bg1, bg0);
        EXPECT_GE(lt1, bg0);
        EXPECT_GT(bg1, bg0);
        EXPECT_GT(lt1, bg0);

        EXPECT_NE(lt0, bg1);
        EXPECT_NE(lt0, lt1);
        EXPECT_LT(lt0, bg1);
        EXPECT_LT(lt0, lt1);
        EXPECT_LE(lt0, bg1);
        EXPECT_LE(lt0, lt1);
        EXPECT_GE(bg1, lt0);
        EXPECT_GE(lt1, lt0);
        EXPECT_GT(bg1, lt0);
        EXPECT_GT(lt1, lt0);
    }

    {
        big_uint16_t bg1{0x1334};
        little_uint16_t lt1{0x1334};

        EXPECT_NE(bg0, bg1);
        EXPECT_NE(bg0, lt1);
        EXPECT_LT(bg0, bg1);
        EXPECT_LT(bg0, lt1);
        EXPECT_LE(bg0, bg1);
        EXPECT_LE(bg0, lt1);
        EXPECT_GE(bg1, bg0);
        EXPECT_GE(lt1, bg0);
        EXPECT_GT(bg1, bg0);
        EXPECT_GT(lt1, bg0);

        EXPECT_NE(lt0, bg1);
        EXPECT_NE(lt0, lt1);
        EXPECT_LT(lt0, bg1);
        EXPECT_LT(lt0, lt1);
        EXPECT_LE(lt0, bg1);
        EXPECT_LE(lt0, lt1);
        EXPECT_GE(bg1, lt0);
        EXPECT_GE(lt1, lt0);
        EXPECT_GT(bg1, lt0);
        EXPECT_GT(lt1, lt0);
    }
}
