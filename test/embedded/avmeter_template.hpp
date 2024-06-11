/*
  UnitTest for UnitADS111x

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_ADS1115_with_EEPROM.hpp>

struct TestParams {
    const bool hal;  // bool true: Using bus false: using wire
    const uint8_t reg;
    const uint8_t reg_eeprom;
};

class TestADS1115 : public ::testing::TestWithParam<TestParams> {
   protected:
    TestADS1115() : ::testing::TestWithParam<TestParams>() {
        auto param = GetParam();
        unit.reset(
            new m5::unit::UnitADS1115WithEEPROM(param.reg, param.reg_eeprom));
        assert(unit);
    }

    virtual void SetUp() override {
        if (!(GetParam().hal) && !wire) {
            Wire.end();
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
            Wire.begin(pin_num_sda, pin_num_scl, 400000U);
            wire = true;
        }

        ustr = m5::utility::formatString("%s:%s", unit->deviceName(),
                                         GetParam().hal ? "Bus" : "Wire");
        // printf("Test as %s\n", ustr.c_str());
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override {
    }

    virtual bool begin() {
        if (GetParam().hal) {
            // Bus
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

            m5::hal::bus::I2CBusConfig i2c_cfg;
            i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
            i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
            auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

            return Units.add(*unit, i2c_bus ? i2c_bus.value() : nullptr) &&
                   Units.begin();
        }
        // Wire
        return Units.add(*unit, Wire) && Units.begin();
    }

    m5::unit::UnitUnified Units;
    std::unique_ptr<m5::unit::UnitADS1115WithEEPROM> unit{};
    std::string ustr{};
    bool wire{};
};

using namespace m5::unit::ads111x;

TEST_P(TestADS1115, GeneralReset) {
    SCOPED_TRACE(ustr);

    // Rewriting config register
    EXPECT_TRUE(unit->setMultiplexer(Mux::AIN_23));
    EXPECT_TRUE(unit->setGain(Gain::PGA_256));
    EXPECT_TRUE(unit->setRate(Rate::SPS_475));
    EXPECT_TRUE(unit->setComparatorQueue(ComparatorQueue::Four));
    EXPECT_TRUE(unit->startPeriodicMeasurement());

    EXPECT_TRUE(unit->generalReset());

    constexpr uint16_t default_value{0x8583};
    EXPECT_EQ(unit->multiplexer(), Mux::AIN_01);
    EXPECT_EQ(unit->gain(), Gain::PGA_2048);
    EXPECT_EQ(unit->rate(), Rate::SPS_128);
    EXPECT_EQ(unit->comparatorQueue(), ComparatorQueue::Disable);

    uint16_t v{};
    EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, v, 0));
    EXPECT_EQ(v, default_value);

    constexpr int16_t default_high = 0x7FFF;
    constexpr int16_t default_low  = 0x8000;
    int16_t high{}, low{};
    EXPECT_TRUE(unit->getThreshould(high, low));
    EXPECT_EQ(high, default_high);
    EXPECT_EQ(low, default_low);
}

TEST_P(TestADS1115, Configration) {
    SCOPED_TRACE(ustr);

    uint16_t prev{}, now{};
    {
        constexpr Mux mux_table[] = {
            Mux::GND_0,  Mux::GND_1,  Mux::GND_2,  Mux::GND_3,
            Mux::AIN_01, Mux::AIN_03, Mux::AIN_13, Mux::AIN_23,
        };

        SCOPED_TRACE("Mux");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : mux_table) {
            EXPECT_TRUE(unit->setMultiplexer(e));

            EXPECT_EQ(unit->multiplexer(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    {
        constexpr Gain gain_table[] = {
            Gain::PGA_6144, Gain::PGA_4096, Gain::PGA_2048,
            Gain::PGA_1024, Gain::PGA_512,  Gain::PGA_256,
        };

        SCOPED_TRACE("Gain");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : gain_table) {
            EXPECT_TRUE(unit->setGain(e));

            EXPECT_EQ(unit->gain(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    {
        SCOPED_TRACE("Mode");
        EXPECT_TRUE(unit->stopPeriodicMeasurement());

        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        EXPECT_TRUE(unit->startPeriodicMeasurement());
        EXPECT_TRUE(unit->inPeriodic());
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
        EXPECT_NE(now, prev);
        prev = now;

        EXPECT_TRUE(unit->stopPeriodicMeasurement());
        EXPECT_FALSE(unit->inPeriodic());
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
        EXPECT_NE(now, prev);
    }

    {
        constexpr Rate rate_table[] = {
            Rate::SPS_8,   Rate::SPS_16,  Rate::SPS_32,  Rate::SPS_64,
            Rate::SPS_128, Rate::SPS_250, Rate::SPS_475, Rate::SPS_860,
        };

        SCOPED_TRACE("Rate");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : rate_table) {
            EXPECT_TRUE(unit->setRate(e));

            EXPECT_EQ(unit->rate(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    constexpr bool bool_table[] = {true, false};
    {
        SCOPED_TRACE("COMP_MODE");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : bool_table) {
            EXPECT_TRUE(unit->setComparatorMode(e));

            EXPECT_EQ(unit->comparatorMode(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    {
        SCOPED_TRACE("COMP_POL");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : bool_table) {
            EXPECT_TRUE(unit->setComparatorPolarity(e));

            EXPECT_EQ(unit->comparatorPolarity(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    {
        SCOPED_TRACE("COMP_LAT");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : bool_table) {
            EXPECT_TRUE(unit->setLatchingComparator(e));

            EXPECT_EQ(unit->latchingComparator(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }

    {
        constexpr ComparatorQueue que_table[] = {
            ComparatorQueue::One,
            ComparatorQueue::Two,
            ComparatorQueue::Four,
            ComparatorQueue::Disable,
        };

        SCOPED_TRACE("COMP_QUE");
        EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, prev, 0));

        for (auto&& e : que_table) {
            EXPECT_TRUE(unit->setComparatorQueue(e));

            EXPECT_EQ(unit->comparatorQueue(), e);

            EXPECT_TRUE(unit->readRegister16(command::CONFIG_REG, now, 0));
            EXPECT_NE(now, prev);
            prev = now;
        }
    }
}

TEST_P(TestADS1115, Periodic) {
    SCOPED_TRACE(ustr);

    std::tuple<const char*, Rate, std::chrono::milliseconds> table[] = {
        {"8sps", Rate::SPS_8, std::chrono::milliseconds(1000 / 8 + 1)},
        {"16sps", Rate::SPS_16, std::chrono::milliseconds(1000 / 16 + 1)},
        {"32sps", Rate::SPS_32, std::chrono::milliseconds(1000 / 32 + 1)},
        {"64sps", Rate::SPS_64, std::chrono::milliseconds(1000 / 64 + 1)},
        {"128sps", Rate::SPS_128, std::chrono::milliseconds(1000 / 128 + 1)},
        {"250sps", Rate::SPS_250, std::chrono::milliseconds(1000 / 250 + 1)},
        {"475sps", Rate::SPS_475, std::chrono::milliseconds(1000 / 475 + 1)},
        {"860sps", Rate::SPS_860, std::chrono::milliseconds(1000 / 860 + 1)},
    };

    {
        // float correction = unit->resolution() * unit->calibrationFactor();

        for (auto&& e : table) {
            const char* s{};
            Rate rate{};
            std::chrono::milliseconds timeout{};
            std::tie(s, rate, timeout) = e;

            SCOPED_TRACE(s);

            EXPECT_TRUE(unit->stopPeriodicMeasurement());
            EXPECT_TRUE(unit->setRate(rate));
            EXPECT_TRUE(unit->startPeriodicMeasurement());

            auto now        = std::chrono::steady_clock::now();
            auto timeout_at = now + timeout;
            uint8_t count{0};
            // Between first and second mesured
            do {
                unit->update();
                bool upd = unit->updated();
                count += upd ? 1 : 0;
                now = std::chrono::steady_clock::now();
                if (upd && count == 1) {  // First?
                    timeout_at = now + timeout;
                }
                std::this_thread::yield();
            } while (count < 2 && now <= timeout_at);

            EXPECT_EQ(count, 2);
            EXPECT_LE(now, timeout_at) << (int64_t)(now - timeout_at).count();
        }
    }
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
}

TEST_P(TestADS1115, SingleShot) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    //    float correction = unit->resolution() * unit->calibrationFactor();

    int cnt{16};
    while (cnt--) {
        int16_t raw{};
        EXPECT_TRUE(unit->readSingleMeasurement(raw));
        //        M5_LOGI("raw:%d current:%f", raw, raw * correction);
    }
}
