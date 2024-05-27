/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

TEST(Utility, Conversion) {
    {
        SCOPED_TRACE("8bit:4");
        std::pair<uint8_t, int8_t> table[] = {
            // zero
            {0x00, 0},
            // only sign bit
            {0x08, -8},
            // all bits
            {0x0F, -1},
            // all signed bits
            {0x07, 7},
        };
        for (auto&& e : table) {
            EXPECT_EQ(m5::utility::unsigned_to_signed<4>(e.first), e.second)
                << e.first;
        }
    }

    {
        SCOPED_TRACE("8bit:8");
        std::pair<uint8_t, int8_t> table[] = {
            // zero
            {0, 0},
            // only sign bit
            {0x80, -128},
            // all bits
            {0xFF, -1},
            // all signed bits
            {0x7F, 127},
        };
        for (auto&& e : table) {
            EXPECT_EQ(m5::utility::unsigned_to_signed<8>(e.first), e.second)
                << e.first;
        }
    }

    {
        SCOPED_TRACE("16bit:10");
        std::pair<uint16_t, int16_t> table[] = {
            // zero
            {0x0000, 0},
            // only sign bit
            {0x0200, -512},
            // all bits
            {0x03FF, -1},
            // all signed bits
            {0x01FF, 511},
            //
            {0x0100, 256},
        };
        for (auto&& e : table) {
            EXPECT_EQ(m5::utility::unsigned_to_signed<10>(e.first), e.second)
                << e.first;
        }
    }

    {
        SCOPED_TRACE("32bit:20");
        std::pair<uint32_t, int32_t> table[] = {
            // zero
            {0x00000000, 0},
            // only sign bit
            {0x00080000, -524288},
            // all bits
            {0x000FFFFF, -1},
            // all signed bits
            {0x0007FFFF, 524287},
            //
            {0x00040000, 262144},
        };
        for (auto&& e : table) {
            EXPECT_EQ(m5::utility::unsigned_to_signed<20>(e.first), e.second)
                << e.first;
        }
    }

    {
        SCOPED_TRACE("64bit:47");
        std::pair<uint64_t, int64_t> table[] = {
            // zero
            {0x0000000000000000ULL, 0},
            // only sign bit
            {0x0000400000000000ULL, -70368744177664LL},
            // all bits
            {0x00007FFFFFFFFFFFULL, -1LL},
            // all signed bits
            {0x00003FFFFFFFFFFFULL, 70368744177663LL},
            //
            {0x0000200000000000ULL, 35184372088832},
        };
        for (auto&& e : table) {
            EXPECT_EQ(m5::utility::unsigned_to_signed<47>(e.first), e.second)
                << e.second;
        }
    }
}
