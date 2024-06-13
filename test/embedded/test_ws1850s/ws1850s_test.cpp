/*
  UnitTest for UnitWS1850S

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_WS1850S.hpp>
#include <M5Utility.hpp>
#include <chrono>
#include <cmath>
#include <array>

namespace {}  // namespace

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::mfrc522;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestWS1850S : public ComponentTestBase<UnitWS1850S, bool> {
   protected:
    virtual UnitWS1850S* get_instance() override {
        return new m5::unit::UnitWS1850S();
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestWS1850S,
//                          ::testing::Values(false, true));
//   INSTANTIATE_TEST_SUITE_P(ParamValues, TestWS1850S,
//   ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestWS1850S, ::testing::Values(false));

using namespace m5::unit::mfrc522;

TEST_P(TestWS1850S, selfTest) {
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->selfTest());  // WS1850S failed always

    Error err{};
    EXPECT_TRUE(unit->getLatestErrorStatus(err));
    EXPECT_EQ(err.value, 0U);
    // M5_LOGW("ERR:%x", err.value);
}

TEST_P(TestWS1850S, coporcessorCRC) {
    const std::array<uint8_t, 8> tdata{
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    };

    struct Table {
        const char* name;
        uint8_t mode;  // ModeReg
        uint16_t init, poly;
        bool refIn, refOut;
        uint16_t xorout;
    };
    constexpr Table table[] = {
        // -- LSB first --
        // CRC-16/KERMIT
        {"0x3C", 0x3C, 0x0000, 0x1021, true, true, 0x0000},
        // CRC-A [ISO/IEC14443-3 Type A]
        {"0x3D", 0x3D, 0xC6C6, 0x1021, true, true, 0x0000},  // reverse 0x6363
        // ???
        {"0x3E", 0x3E, 0x8E65, 0x1021, true, true, 0x0000},  // reverse 0xA671
        // // CRC-16/MCRF4XX
        {"0x3F", 0x3F, 0xFFFF, 0x1021, true, true, 0x0000},
        // -- MSB first --
        {"0xBC", 0xBC, 0x0000, 0x1021, false, false, 0x0000},
        {"0xBD", 0xBD, 0xC6C6, 0x1021, false, false, 0x0000},
        {"0xBE", 0xBE, 0x8E65, 0x1021, false, false, 0x0000},
        {"0xBF", 0xBF, 0xFFFF, 0x1021, false, false, 0x0000},
    };

    // Change preset value for CRC coprocessor and check CRC values
    for (auto&& e : table) {
        unit->writeRegister8(m5::unit::mfrc522::command::MODE_REG, e.mode);

        m5::utility::CRC16 crc(e.init, e.poly, e.refIn, e.refOut, e.xorout);
        uint16_t result{};
        EXPECT_TRUE(unit->calculateCRC(tdata.data(), tdata.size(), result))
            << e.name;

        uint16_t cr = crc.get(tdata.data(), tdata.size());
        EXPECT_EQ(result, cr) << e.name;
        if (result != cr) {
            M5_LOGI("%s >>>> %x | %x", e.name, result, cr);
        }
    }
}

TEST_P(TestWS1850S, Antenna) {
    SCOPED_TRACE(ustr);

    uint8_t prev{}, now{};
    bool onoff{};

    // on after begin
    EXPECT_TRUE(unit->isAntennaOn(onoff));
    EXPECT_TRUE(onoff);

    // to ON
    EXPECT_TRUE(unit->readRegister8(m5::unit::mfrc522::command::TX_CONTROL_REG,
                                    prev, 0));

    EXPECT_TRUE(unit->turnOffAntenna());
    EXPECT_TRUE(unit->readRegister8(m5::unit::mfrc522::command::TX_CONTROL_REG,
                                    now, 0));
    EXPECT_TRUE(unit->isAntennaOn(onoff));
    EXPECT_FALSE(onoff);
    EXPECT_NE(now, prev);
    prev = now;

    // to OFF
    EXPECT_TRUE(unit->turnOnAntenna());
    EXPECT_TRUE(unit->readRegister8(m5::unit::mfrc522::command::TX_CONTROL_REG,
                                    now, 0));
    EXPECT_TRUE(unit->isAntennaOn(onoff));
    EXPECT_TRUE(onoff);
    EXPECT_NE(now, prev);

    // Change gain
    constexpr ReceiverGain table[] = {
        ReceiverGain::dB18, ReceiverGain::dB23, ReceiverGain::dB33,
        ReceiverGain::dB38, ReceiverGain::dB43, ReceiverGain::dB48,
    };

    EXPECT_TRUE(
        unit->readRegister8(m5::unit::mfrc522::command::RFC_FG_REG, prev, 0));

    for (auto&& e : table) {
        EXPECT_TRUE(unit->setAntennaGain(e)) << (int)e;
        EXPECT_TRUE(unit->readRegister8(m5::unit::mfrc522::command::RFC_FG_REG,
                                        now, 0));

        ReceiverGain gain{};
        EXPECT_TRUE(unit->getAntennaGain(gain)) << (int)e;
        EXPECT_EQ(gain, e) << (int)e;
        EXPECT_NE(now, prev) << (int)e;
        prev = now;
    }
}

TEST_P(TestWS1850S, Power) {
    // TODO
#if 0
    SCOPED_TRACE(ustr);

    uint8_t now{}, prev{};

    EXPECT_TRUE(
        unit->readRegister8(m5::unit::mfrc522::command::COMMAND_REG, prev, 0));
    M5_LOGW("prev:%x", prev);

    // power down
    EXPECT_TRUE(unit->enablePowerDownMode());

    EXPECT_TRUE(
        unit->readRegister8(m5::unit::mfrc522::command::COMMAND_REG, now, 0));
    EXPECT_EQ((now & 0x10), 0x10);
    EXPECT_NE(now, prev);
    prev = now;

    // powerup
    EXPECT_TRUE(unit->disablePowerDownMode());

    EXPECT_TRUE(
        unit->readRegister8(m5::unit::mfrc522::command::COMMAND_REG, now, 0));
    EXPECT_EQ((now & 0x10), 0x00);
    EXPECT_NE(now, prev);

    M5_LOGW("now:%x", now);
#endif
}
