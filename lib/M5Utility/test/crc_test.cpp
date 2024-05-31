/*
  UnitTest for M5Utility

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>

namespace {
struct CRCType {
    const char* name;
    const uint32_t result;
    const uint32_t poly;
    const uint32_t init;
    const bool refIn;
    const bool refOut;
    const uint32_t xorout;
};

// https://crccalc.com/?crc=0123456789abcdef&method=crc8&datatype=hex&outtype=0
CRCType crc8_table[] = {
    // name, result, poly, init, refIn, refOut, xorout
    {"CRC-8", 0x1E, 0x07, 0x00, false, false, 0x00},
    {"CRC-8/CDMA2000", 0xF0, 0x9B, 0xFF, false, false, 0x00},
    {"CRC-8/DARC", 0x4E, 0x39, 0x00, true, true, 0x00},
    {"CRC-8/DVB-S2", 0x7D, 0xD5, 0x00, false, false, 0x00},
    {"CRC-8/EBU", 0x17, 0x1D, 0xFF, true, true, 0x00},
    {"CRC-8/I-CODE", 0x6C, 0x1D, 0xFD, false, false, 0x00},
    {"CRC-8/ITU", 0x4B, 0x07, 0x00, false, false, 0x55},
    {"CRC-8/MAXIM", 0xDD, 0x31, 0x00, true, true, 0x00},
    {"CRC-8/ROHC", 0xFC, 0x07, 0xFF, true, true, 0x00},
    {"CRC-8/WCDMA", 0x22, 0x9B, 0x00, true, true, 0x00},
};

CRCType crc16_table[] = {
    // name, result, poly, init, refIn, refOut, xorout
    {"CRC-16/ARC", 0xF3A6, 0x8005, 0x0000, true, true, 0x0000},
    {"CRC-16/AUG-CCITT", 0x486C, 0x1021, 0x1D0F, false, false, 0x0000},
    {"CRC-16/BUYPASS", 0x2951, 0x8005, 0x0000, false, false, 0x0000},
    {"CRC-16/CCITT-FALSE", 0x986B, 0x1021, 0xFFFF, false, false, 0x0000},
    {"CRC-16/CDMA2000", 0x593C, 0xC867, 0xFFFF, false, false, 0x0000},
    {"CRC-16/DDS-110", 0x27B1, 0x8005, 0x800D, false, false, 0x0000},
    {"CRC-16/DECT-R", 0xE83B, 0x0589, 0x0000, false, false, 0x0001},
    {"CRC-16/DECT-X", 0xE83A, 0x0589, 0x0000, false, false, 0x0000},
    {"CRC-16/DNP", 0xDFCE, 0x3D65, 0x0000, true, true, 0xFFFF},
    {"CRC-16/EN-13757", 0x984F, 0x3D65, 0x0000, false, false, 0xFFFF},
    {"CRC-16/GENIBUS", 0x6794, 0x1021, 0xFFFF, false, false, 0xFFFF},
    {"CRC-16/KERMIT", 0x5A3C, 0x1021, 0x0000, true, true, 0x0000},
    {"CRC-16/MAXIM", 0x0C59, 0x8005, 0x0000, true, true, 0xFFFF},
    {"CRC-16/MCRF4XX", 0x26B0, 0x1021, 0xFFFF, true, true, 0x0000},
    {"CRC-16/MODBUS", 0xF8E6, 0x8005, 0xFFFF, true, true, 0x0000},
    {"CRC-16/RIELLO", 0x077C, 0x1021, 0xB2AA, true, true, 0x0000},  // 0x554D
    {"CRC-16/T10-DIF", 0x4FF0, 0x8BB7, 0x0000, false, false, 0x0000},
    {"CRC-16/TELEDISK", 0x3267, 0xA097, 0x0000, false, false, 0x0000},
    {"CRC-16/TMS37157", 0x5A4B, 0x1021, 0x89EC, true, true, 0x0000},  // 0x3791
    {"CRC-16/USB", 0x0719, 0x8005, 0xFFFF, true, true, 0xFFFF},
    {"CRC-16/X-25", 0xD94F, 0x1021, 0xFFFF, true, true, 0xFFFF},
    {"CRC-16/XMODEM", 0xA955, 0x1021, 0x0000, false, false, 0x0000},
    {"CRC-A", 0x0F06, 0x1021, 0xC6C6, true, true, 0x0000},  // 0x6363
};

const std::vector<uint8_t> tdata = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

};  // namespace

using namespace m5::utility;

TEST(Utility, CRC8) {
    for (auto&& e : crc8_table) {
        CRC8 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        EXPECT_EQ(crc.get(tdata.data(), tdata.size()), e.result) << e.name;
    }
}

TEST(Utility, CRC16) {
    for (auto&& e : crc16_table) {
        CRC16 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        EXPECT_EQ(crc.get(tdata.data(), tdata.size()), e.result) << e.name;
    }
}
