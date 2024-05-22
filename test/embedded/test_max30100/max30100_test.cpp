/*
  UnitTest for UnitMAX30100

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_MAX30100.hpp>
#include <chrono>
#include <cmath>
#include <iostream>

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

// bool true: Using bus false: using wire
class TestMAX30100 : public ::testing::TestWithParam<bool> {
   protected:
    virtual void SetUp() override {
        if (!GetParam()) {
            Wire.end();
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
            Wire.begin(pin_num_sda, pin_num_scl, 400000U);
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
    m5::unit::UnitMAX30100 unit;
    std::string ustr{};
};

// true:Bus false:Wire
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100,
//                         ::testing::Values(true, false));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100, ::testing::Values(false));

namespace {
using namespace m5::unit::max30100;
// See also Table.8 & 9
bool is_allowed_settings(const Mode mode, const SamplingRate rate,
                         const LedPulseWidth pw) {
    constexpr uint8_t spo2_table[] = {
        // LSB:200 MSG:1600
        0x0F, 0x0F, 0x07, 0x07, 0x03, 0x01, 0x01, 0x01,
    };
    constexpr uint8_t hr_table[] = {
#if 0
        // LSB:200 MSG:1600
        0x0F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x03, 0x03,
#else
        // The description on the data sheet is as above,
        // but some invalid values can be set. (bug or spec?)
        // LSB:200 MSG:1600
        0x0F, 0x0F, 0x0F, 0x0F, 0x07, 0x03, 0x03, 0x03,
#endif
    };
    return (mode == Mode::SPO2 ? spo2_table
                               : hr_table)[m5::stl::to_underlying(rate)] &
           (1U << m5::stl::to_underlying(pw));
}

void initialize(m5::unit::UnitMAX30100& u) {
    EXPECT_TRUE(u.reset());

#if 1
    // set SPO2, Sampling50, PW200, Current50
    EXPECT_TRUE(u.setMode(m5::unit::max30100::Mode::SPO2));

    m5::unit::max30100::SpO2Configuration sc{};
    sc.samplingRate(m5::unit::max30100::SamplingRate::Sampling50);
    sc.ledPulseWidth(m5::unit::max30100::LedPulseWidth::PW200);
    sc.highResolution(true);
    EXPECT_TRUE(u.setSpO2Configuration(sc));

    EXPECT_TRUE(u.getSpO2Configuration(sc));
    EXPECT_TRUE(sc.highResolution());
    EXPECT_EQ(sc.samplingRate(), m5::unit::max30100::SamplingRate::Sampling50);
    EXPECT_EQ(sc.ledPulseWidth(), m5::unit::max30100::LedPulseWidth::PW200);

    EXPECT_TRUE(u.setLedCurrent(m5::unit::max30100::CurrentControl::mA50_0,
                                m5::unit::max30100::CurrentControl::mA50_0));
    m5::unit::max30100::LedConfiguration lc{};
    EXPECT_TRUE(u.getLedConfiguration(lc));
    EXPECT_EQ(lc.irLed(), m5::unit::max30100::CurrentControl::mA50_0);
    EXPECT_EQ(lc.redLed(), m5::unit::max30100::CurrentControl::mA50_0);
#endif
}

}  // namespace

TEST_P(TestMAX30100, Configration) {
    SCOPED_TRACE(ustr);

    initialize(unit);

    {
        constexpr m5::unit::max30100::Mode table[] = {
            m5::unit::max30100::Mode::SPO2,
            m5::unit::max30100::Mode::HROnly,
        };

        for (auto&& m : table) {
            m5::unit::max30100::ModeConfiguration mc{};

            EXPECT_TRUE(unit.setMode(m)) << (int)m;

            EXPECT_TRUE(unit.getModeConfiguration(mc)) << (int)m;
            EXPECT_EQ(mc.mode(), m) << (int)m;

            auto mm = (m == m5::unit::max30100::Mode::HROnly
                           ? m5::unit::max30100::Mode::SPO2
                           : m5::unit::max30100::Mode::HROnly);
            mc.mode(mm);
            EXPECT_TRUE(unit.setModeConfiguration(mc)) << (int)m;

            EXPECT_TRUE(unit.getModeConfiguration(mc)) << (int)m;
            EXPECT_EQ(mc.mode(), mm) << (int)m;
        }

        constexpr bool ps_table[] = {true, false};
        for (auto&& ps : ps_table) {
            m5::unit::max30100::ModeConfiguration mc{};

            EXPECT_TRUE(ps ? unit.enablePowerSave() : unit.disablePowerSave())
                << ps;

            EXPECT_TRUE(unit.getModeConfiguration(mc)) << ps;
            EXPECT_EQ(mc.shdn(), ps) << ps;

            bool f = !ps;
            mc.shdn(f);
            EXPECT_TRUE(unit.setModeConfiguration(mc)) << ps;

            EXPECT_TRUE(unit.getModeConfiguration(mc)) << ps;
            EXPECT_EQ(mc.shdn(), f) << ps;
        }
    }

    {
        constexpr m5::unit::max30100::SamplingRate sr_table[] = {
            m5::unit::max30100::SamplingRate::Sampling50,
            m5::unit::max30100::SamplingRate::Sampling100,
            m5::unit::max30100::SamplingRate::Sampling167,
            m5::unit::max30100::SamplingRate::Sampling200,
            m5::unit::max30100::SamplingRate::Sampling400,
            m5::unit::max30100::SamplingRate::Sampling600,
            m5::unit::max30100::SamplingRate::Sampling800,
            m5::unit::max30100::SamplingRate::Sampling1000,
        };
        constexpr m5::unit::max30100::LedPulseWidth pw_table[] = {
            m5::unit::max30100::LedPulseWidth::PW200,
            m5::unit::max30100::LedPulseWidth::PW400,
            m5::unit::max30100::LedPulseWidth::PW800,
            m5::unit::max30100::LedPulseWidth::PW1600,
        };

        {
            SCOPED_TRACE("SPO2");
            initialize(unit);

            EXPECT_TRUE(unit.setMode(m5::unit::max30100::Mode::SPO2));
            for (auto&& rate : sr_table) {
                for (auto&& pw : pw_table) {
                    m5::unit::max30100::SpO2Configuration sc{};
                    EXPECT_TRUE(unit.getSpO2Configuration(sc))
                        << "Rate:" << (int)rate << " PW:" << (int)pw;
                    sc.samplingRate(rate);
                    sc.ledPulseWidth(pw);
                    if (is_allowed_settings(m5::unit::max30100::Mode::SPO2,
                                            rate, pw)) {
                        EXPECT_TRUE(unit.setSpO2Configuration(sc))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_TRUE(unit.setSamplingRate(rate))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_TRUE(unit.setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                    } else {
                        EXPECT_FALSE(unit.setSpO2Configuration(sc))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_FALSE(unit.setSamplingRate(rate))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#if 0
                        // setLedPulseWidth here may succeed because no
                        // deviating data is written in setSamplingRate, so it
                        // is not tested
                        EXPECT_FALSE(unit.setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#endif
                    }
                }
            }
        }
        {
            SCOPED_TRACE("HROnly");
            initialize(unit);

            EXPECT_TRUE(unit.setMode(m5::unit::max30100::Mode::HROnly));
            for (auto&& rate : sr_table) {
                for (auto&& pw : pw_table) {
                    m5::unit::max30100::SpO2Configuration sc{};
                    EXPECT_TRUE(unit.getSpO2Configuration(sc))
                        << "Rate:" << (int)rate << " PW:" << (int)pw;
                    sc.samplingRate(rate);
                    sc.ledPulseWidth(pw);
                    if (is_allowed_settings(m5::unit::max30100::Mode::HROnly,
                                            rate, pw)) {
                        EXPECT_TRUE(unit.setSpO2Configuration(sc))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_TRUE(unit.setSamplingRate(rate))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_TRUE(unit.setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                    } else {
                        EXPECT_FALSE(unit.setSpO2Configuration(sc))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
                        EXPECT_FALSE(unit.setSamplingRate(rate))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#if 0
                        // setLedPulseWidth here may succeed because no
                        // deviating data is written in setSamplingRate, so it
                        // is not tested
                        EXPECT_FALSE(unit.setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#endif
                    }
                }
            }
        }

        {
            constexpr bool hr_table[] = {true, false};

            for (auto&& hr : hr_table) {
                EXPECT_TRUE(hr ? unit.enableHighResolution()
                               : unit.disableHighResolution())
                    << hr;
                m5::unit::max30100::SpO2Configuration sc{};
                EXPECT_TRUE(unit.getSpO2Configuration(sc)) << hr;
                EXPECT_EQ(sc.highResolution(), hr) << hr;
            }
        }
    }

    {
        constexpr m5::unit::max30100::CurrentControl cc_table[] = {
            m5::unit::max30100::CurrentControl::mA0_0,
            m5::unit::max30100::CurrentControl::mA4_4,
            m5::unit::max30100::CurrentControl::mA7_6,
            m5::unit::max30100::CurrentControl::mA11_0,
            m5::unit::max30100::CurrentControl::mA14_2,
            m5::unit::max30100::CurrentControl::mA17_4,
            m5::unit::max30100::CurrentControl::mA20_8,
            m5::unit::max30100::CurrentControl::mA24_0,
            m5::unit::max30100::CurrentControl::mA27_1,
            m5::unit::max30100::CurrentControl::mA30_6,
            m5::unit::max30100::CurrentControl::mA33_8,
            m5::unit::max30100::CurrentControl::mA37_0,
            m5::unit::max30100::CurrentControl::mA40_2,
            m5::unit::max30100::CurrentControl::mA43_6,
            m5::unit::max30100::CurrentControl::mA46_8,
            m5::unit::max30100::CurrentControl::mA50_0,
        };

        for (auto&& ir : cc_table) {
            for (auto&& red : cc_table) {
                m5::unit::max30100::LedConfiguration lc{};

                EXPECT_TRUE(unit.setLedCurrent(ir, red))
                    << "IR:" << (int)ir << " RED:" << (int)red;

                EXPECT_TRUE(unit.getLedConfiguration(lc))
                    << "IR:" << (int)ir << " RED:" << (int)red;

                EXPECT_EQ(lc.irLed(), ir)
                    << "IR:" << (int)ir << " RED:" << (int)red;
                EXPECT_EQ(lc.redLed(), red)
                    << "IR:" << (int)ir << " RED:" << (int)red;

                lc.irLed(ir);
                lc.redLed(red);
                EXPECT_TRUE(unit.setLedConfiguration(lc))
                    << "IR:" << (int)ir << " RED:" << (int)red;

                EXPECT_TRUE(unit.getLedConfiguration(lc))
                    << "IR:" << (int)ir << " RED:" << (int)red;
                EXPECT_EQ(lc.irLed(), ir)
                    << "IR:" << (int)ir << " RED:" << (int)red;
                EXPECT_EQ(lc.redLed(), red)
                    << "IR:" << (int)ir << " RED:" << (int)red;
            }
        }
    }
}

TEST_P(TestMAX30100, Temperature) {
    SCOPED_TRACE(ustr);

    //    initialize(unit);

    //    EXPECT_TRUE(unit.disablePowerSave());
    //    m5::utility::delay(500);

    EXPECT_TRUE(unit.measurementTemperature());

    auto timeout  = std::chrono::seconds(1);
    auto start_at = std::chrono::steady_clock::now();
    auto elapsed  = start_at;
    bool done{};

    do {
        M5_LOGE(">");
        done    = unit.isMeasurementTemperature();
        elapsed = std::chrono::steady_clock::now();
    } while (!done && (elapsed - start_at) <= timeout);

    EXPECT_TRUE(done);
    EXPECT_LE(elapsed - start_at, timeout);

    float temp{};
    EXPECT_TRUE(unit.readMeasurementTemperature(temp));
    EXPECT_TRUE(std::isfinite(temp));

    M5_LOGI("temp:%f", temp);
}
