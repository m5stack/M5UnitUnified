/*
  UnitTest for UnitSHT3x

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_ADS111x.hpp>
#include <chrono>
#include <iostream>
#include <bitset>

#if 0
class GlobalFixture : public ::testing::Environment {
   public:
    void SetUp() override {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
        // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
        Wire.begin(pin_num_sda, pin_num_scl, 400000U);
    }
};
const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture);
#endif

constexpr uint8_t i2c_address{0x48};  // Ameter
// constexpr uint8_t i2c_address{0x49};  // Vmeter

// bool true: Using bus false: using wire
class TestADS1115 : public ::testing::TestWithParam<bool> {
   protected:
    TestADS1115() : ::testing::TestWithParam<bool>(), unit(i2c_address) {
    }

    virtual void SetUp() override {
        if (!GetParam() && !wire) {
            Wire.end();
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
            Wire.begin(pin_num_sda, pin_num_scl, 400000U);
            wire = true;
        }

        ustr = m5::utility::formatString("%s:%s", unit.deviceName(),
                                         GetParam() ? "Bus" : "Wire");
        // printf("Test as %s\n", ustr.c_str());

        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override {
    }

    virtual bool begin() {
        if (GetParam()) {
            // Bus
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

            m5::hal::bus::I2CBusConfig i2c_cfg;
            i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
            i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
            auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

            return Units.add(unit, i2c_bus ? i2c_bus.value() : nullptr) &&
                   Units.begin();
        }
        // Wire
        return Units.add(unit, Wire) && Units.begin();
    }

    m5::unit::UnitUnified Units;
    m5::unit::UnitADS1115 unit;
    std::string ustr{};
    bool wire{};
};

// true:Bus false:Wire
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestADS1115,
//                         ::testing::Values(true, false));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestADS1115, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestADS1115, ::testing::Values(false));

using namespace m5::unit::ads111x;

TEST_P(TestADS1115, GeneralReset) {
    SCOPED_TRACE(ustr);

    // Rewriting config register
    EXPECT_TRUE(unit.setMultiplexer(Mux::AIN_23));
    EXPECT_TRUE(unit.setGain(Gain::PGA_256));
    EXPECT_TRUE(unit.setRate(Rate::SPS_475));
    EXPECT_TRUE(unit.setComparatorQueue(ComparatorQueue::Four));
    EXPECT_TRUE(unit.startPeriodicMeasurement());

    EXPECT_TRUE(unit.generalReset());

    constexpr uint16_t default_value{0x8583};
    EXPECT_EQ(unit.mux(), Mux::AIN_01);
    EXPECT_EQ(unit.gain(), Gain::PGA_2048);
    EXPECT_EQ(unit.rate(), Rate::SPS_128);
    EXPECT_EQ(unit.compQueue(), ComparatorQueue::Disable);

    uint16_t v{};
    EXPECT_TRUE(unit.readRegister16(command::CONFIG_REG, v, 0));
    EXPECT_EQ(v, default_value);
}
