/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

namespace {

struct CRC8_Type {
    const char* name;
    const uint8_t poly;
    const uint8_t xorout;
    const uint8_t init;
    const uint8_t result;
};

CRC8_Type crc8_table[] = {
    {"CRC-8", 0x07, 0x00, 0x00, 0x1E},
    {"CRC-8/CDMA2000", 0x9B, 0x00, 0xFF, 0xF0},
    {"CRC-8/DVB-S2", 0xD5, 0x00, 0x00, 0x7D},
    {"CRC-8/I-CODE", 0x1D, 0x00, 0xFD, 0x6C},
    {"CRC-8/ITU", 0x07, 0x55, 0x00, 0x4B},
    {"CRC-8/MAXIM", 0x31, 0x00, 0x00, 0x05},
};

};  // namespace

TEST(Utility, CRC8) {
    std::vector<uint8_t> data = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    };
    for (auto&& e : crc8_table) {
        EXPECT_EQ(m5::utility::CRC8::calculate(data.data(), data.size(), e.poly,
                                               e.xorout, e.init),
                  e.result)
            << e.name;
    }
}
