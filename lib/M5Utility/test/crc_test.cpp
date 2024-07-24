/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5Utility
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>
#include <M5Unified.hpp>

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

// See also
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
        SCOPED_TRACE(e.name);
        CRC8 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        auto v = crc.range(tdata.data(), tdata.size());
        auto u = crc.update(tdata.data(), tdata.size());
        EXPECT_EQ(v, e.result);
        EXPECT_EQ(crc.value(), e.result);
        EXPECT_EQ(u, crc.value());

        v = crc.range(tdata.data(), tdata.size());
        u = crc.update(tdata.data(), tdata.size());
        EXPECT_EQ(v, e.result);
        EXPECT_NE(v, u);
    }
}

TEST(Utility, CRC16) {
    for (auto&& e : crc16_table) {
        SCOPED_TRACE(e.name);
        CRC16 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        auto v = crc.update(tdata.data(), tdata.size());
        EXPECT_EQ(crc.value(), e.result);
        EXPECT_EQ(v, crc.value());
    }
}

// Test whether calculation from the whole and calculation from split chunks are
// equivalent
TEST(Utility, Chunk) {
    constexpr uint8_t d8[32] = {0x04, 0x67, 0xfc, 0x4d, 0xf4, 0xe7, 0x9c, 0x3b,
                                0x05, 0xb8, 0xad, 0x31, 0x97, 0xb1, 0x21, 0x72,
                                0x59, 0x5d, 0x80, 0x26, 0x66, 0x0c, 0x12, 0xa9,
                                0x53, 0xa6, 0x70, 0x87, 0x91, 0x5d, 0xa4, 0x9a};
    ;

    // CRC8
    for (auto&& e : crc8_table) {
        SCOPED_TRACE(e.name);
        uint8_t crc_all = CRC8::calculate(d8, m5::stl::size(d8), e.init, e.poly,
                                          e.refIn, e.refOut, e.xorout);

        CRC8 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        uint8_t crc_chunk{};
        for (int i = 0; i < 4; ++i) {
            crc_chunk = crc.update(d8 + (i * 8), 8);
            // M5_LOGW("%s:i:%d:[%x/%x]", e.name, i, crc_chunk,
            //         crc.range(d8 + (i * 8), 8));
            if (i == 0) {
                EXPECT_EQ(crc_chunk, crc.range(d8 + (i * 8), 8));
            } else {
                EXPECT_NE(crc_chunk, crc.range(d8 + (i * 8), 8));
            }
        }
        EXPECT_EQ(crc_all, crc_chunk);
        EXPECT_EQ(crc_all, crc.value());
    }

    // CRC16
    for (auto&& e : crc16_table) {
        SCOPED_TRACE(e.name);
        uint16_t crc_all = CRC16::calculate(
            d8, m5::stl::size(d8), e.init, e.poly, e.refIn, e.refOut, e.xorout);

        CRC16 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        uint16_t crc_chunk{};
        for (int i = 0; i < 4; ++i) {
            crc_chunk = crc.update(d8 + (i * 8), 8);
            if (i == 0) {
                EXPECT_EQ(crc_chunk, crc.range(d8 + (i * 8), 8));
            } else {
                EXPECT_NE(crc_chunk, crc.range(d8 + (i * 8), 8));
            }
        }
        EXPECT_EQ(crc_all, crc_chunk);
        EXPECT_EQ(crc_all, crc_chunk);
    }
}
