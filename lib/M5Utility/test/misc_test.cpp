/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

using namespace m5::utility;

TEST(Utility, reverseBitOrder) {
    EXPECT_EQ(reverseBitOrder((uint8_t)0), 0);
    EXPECT_EQ(reverseBitOrder((uint16_t)0), 0);

    EXPECT_EQ(reverseBitOrder((uint8_t)0xFF), 0xFF);
    EXPECT_EQ(reverseBitOrder((uint16_t)0xFFFF), 0xFFFF);

    EXPECT_EQ(reverseBitOrder((uint8_t)0x0F), 0xF0);
    EXPECT_EQ(reverseBitOrder((uint16_t)0xFF00), 0x00FF);

    EXPECT_EQ(reverseBitOrder((uint8_t)0x4C), 0x32);
    EXPECT_EQ(reverseBitOrder((uint16_t)0x4C4C), 0x3232);

    EXPECT_EQ(reverseBitOrder((uint8_t)0x8E), 0x71);
    EXPECT_EQ(reverseBitOrder((uint8_t)0x65), 0xA6);
    EXPECT_EQ(reverseBitOrder((uint16_t)0x8E65), 0xA671);
}
