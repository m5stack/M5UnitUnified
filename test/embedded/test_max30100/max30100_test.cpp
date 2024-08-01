/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitMAX30100
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_MAX30100.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::max30100;
using namespace m5::unit::max30100::command;

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestMAX30100 : public ComponentTestBase<UnitMAX30100, bool> {
   protected:
    virtual UnitMAX30100* get_instance() override {
        auto ptr = new m5::unit::UnitMAX30100();
        if (ptr) {
            auto cfg = ptr->config();
            ptr->config(cfg);
        }
        return ptr;
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100,
//                         ::testing::Values(false, true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100,
// ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestMAX30100, ::testing::Values(false));

namespace {
// See also Table.8 & 9
bool is_allowed_settings(const Mode mode, const Sampling rate,
                         const LedPulseWidth pw) {
    constexpr uint8_t spo2_table[] = {
        // LSB:200 MSG:1600
        0x0F, 0x0F, 0x07, 0x07, 0x03, 0x01, 0x01, 0x01,
    };
    constexpr uint8_t hr_table[] = {
        // LSB:200 MSG:1600
        // 0x0F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x03, 0x03,
        // LSB:200 MSG:1600
        0x0F, 0x0F, 0x0F, 0x0F, 0x07, 0x03, 0x03, 0x03,
        // The description on the data sheet is as above,
        // but some invalid values can be set. (bug or spec?)
    };
    return (mode == Mode::SPO2 ? spo2_table
                               : hr_table)[m5::stl::to_underlying(rate)] &
           (1U << m5::stl::to_underlying(pw));
}

constexpr Mode mode_table[] = {
    Mode::SPO2,
    Mode::HROnly,
};

}  // namespace

TEST_P(TestMAX30100, Configration) {
    SCOPED_TRACE(ustr);

    {
        for (auto&& m : mode_table) {
            auto s = m5::utility::formatString("Mode:%u", m);
            SCOPED_TRACE(s.c_str());
            // M5_LOGI("%s", s.c_str());

            ModeConfiguration mc{};
            EXPECT_TRUE(unit->setMode(m));

            EXPECT_TRUE(unit->readModeConfiguration(mc));
            EXPECT_EQ(mc.mode(), m);

            auto mm = (m == Mode::HROnly ? Mode::SPO2 : Mode::HROnly);
            mc.mode(mm);
            EXPECT_TRUE(unit->setModeConfiguration(mc));

            EXPECT_TRUE(unit->readModeConfiguration(mc));
            EXPECT_EQ(mc.mode(), mm);
        }

        constexpr bool ps_table[] = {true, false};
        for (auto&& ps : ps_table) {
            auto s = m5::utility::formatString("PowerSave:%u", ps);
            SCOPED_TRACE(s.c_str());
            // M5_LOGI("%s", s.c_str());

            ModeConfiguration mc{};

            EXPECT_TRUE(ps ? unit->enablePowerSave()
                           : unit->disablePowerSave());
            EXPECT_TRUE(unit->readModeConfiguration(mc));
            EXPECT_EQ(mc.shdn(), ps);

            bool f = !ps;
            mc.shdn(f);
            EXPECT_TRUE(unit->setModeConfiguration(mc));

            EXPECT_TRUE(unit->readModeConfiguration(mc));
            EXPECT_EQ(mc.shdn(), f);
        }
    }

    {
        constexpr Sampling sr_table[] = {
            Sampling::Rate50,  Sampling::Rate100,  Sampling::Rate167,
            Sampling::Rate200, Sampling::Rate400,  Sampling::Rate600,
            Sampling::Rate800, Sampling::Rate1000,
        };
        constexpr LedPulseWidth pw_table[] = {
            LedPulseWidth::PW200,
            LedPulseWidth::PW400,
            LedPulseWidth::PW800,
            LedPulseWidth::PW1600,
        };

        {
            EXPECT_TRUE(unit->setMode(Mode::SPO2));
            for (auto&& rate : sr_table) {
                for (auto&& pw : pw_table) {
                    auto s = m5::utility::formatString("SPO2:Rate:%u:LPW:%u",
                                                       rate, pw);
                    SCOPED_TRACE(s.c_str());
                    // M5_LOGI("%s", s.c_str());

                    SpO2Configuration sc{};
                    EXPECT_TRUE(unit->readSpO2Configuration(sc));
                    sc.samplingRate(rate);
                    sc.ledPulseWidth(pw);
                    if (is_allowed_settings(Mode::SPO2, rate, pw)) {
                        EXPECT_TRUE(unit->setSpO2Configuration(sc));
                        EXPECT_TRUE(unit->setSamplingRate(rate));
                        EXPECT_TRUE(unit->setLedPulseWidth(pw));
                    } else {
                        EXPECT_FALSE(unit->setSpO2Configuration(sc));
                        EXPECT_FALSE(unit->setSamplingRate(rate));
#if 0
                        // setLedPulseWidth here may succeed because no
                        // deviating data is written in setSamplingRate, so it
                        // is not tested
                        EXPECT_FALSE(unit->setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#endif
                    }
                }
            }
        }
        {
            EXPECT_TRUE(unit->setMode(Mode::HROnly));
            for (auto&& rate : sr_table) {
                for (auto&& pw : pw_table) {
                    auto s = m5::utility::formatString("HRONly:Rate:%u:LPW:%u",
                                                       rate, pw);
                    SCOPED_TRACE(s.c_str());
                    // M5_LOGI("%s", s.c_str());

                    SpO2Configuration sc{};
                    EXPECT_TRUE(unit->readSpO2Configuration(sc));
                    sc.samplingRate(rate);
                    sc.ledPulseWidth(pw);
                    if (is_allowed_settings(Mode::HROnly, rate, pw)) {
                        EXPECT_TRUE(unit->setSpO2Configuration(sc));
                        EXPECT_TRUE(unit->setSamplingRate(rate));
                        EXPECT_TRUE(unit->setLedPulseWidth(pw));
                    } else {
                        EXPECT_FALSE(unit->setSpO2Configuration(sc));
                        EXPECT_FALSE(unit->setSamplingRate(rate));
#if 0
                        // setLedPulseWidth here may succeed because no
                        // deviating data is written in setSamplingRate, so it
                        // is not tested
                        EXPECT_FALSE(unit->setLedPulseWidth(pw))
                            << "Rate:" << (int)rate << " PW:" << (int)pw;
#endif
                    }
                }
            }
        }

        {
            constexpr bool hr_table[] = {true, false};

            for (auto&& hr : hr_table) {
                auto s = m5::utility::formatString("HightRes:%u", hr);
                SCOPED_TRACE(s.c_str());
                // M5_LOGI("%s", s.c_str());

                EXPECT_TRUE(hr ? unit->enableHighResolution()
                               : unit->disableHighResolution());

                SpO2Configuration sc{};
                EXPECT_TRUE(unit->readSpO2Configuration(sc));
                EXPECT_EQ(sc.highResolution(), hr);
            }
        }
    }

    {
        constexpr CurrentControl cc_table[] = {
            CurrentControl::mA0_0,  CurrentControl::mA4_4,
            CurrentControl::mA7_6,  CurrentControl::mA11_0,
            CurrentControl::mA14_2, CurrentControl::mA17_4,
            CurrentControl::mA20_8, CurrentControl::mA24_0,
            CurrentControl::mA27_1, CurrentControl::mA30_6,
            CurrentControl::mA33_8, CurrentControl::mA37_0,
            CurrentControl::mA40_2, CurrentControl::mA43_6,
            CurrentControl::mA46_8, CurrentControl::mA50_0,
        };

        // In the heart-rate only mode, the red LED is inactive,
        {
            EXPECT_TRUE(unit->setMode(Mode::SPO2));
            for (auto&& ir : cc_table) {
                for (auto&& red : cc_table) {
                    auto s =
                        m5::utility::formatString("SPO2:IR:%u/RED:%u", ir, red);
                    SCOPED_TRACE(s.c_str());
                    // M5_LOGI("%s", s.c_str());

                    LedConfiguration lc{};

                    EXPECT_TRUE(unit->setLedCurrent(ir, red));

                    EXPECT_TRUE(unit->readLedConfiguration(lc));
                    EXPECT_EQ(lc.irLed(), ir);
                    EXPECT_EQ(lc.redLed(), red);

                    lc.irLed(ir);
                    lc.redLed(red);
                    EXPECT_TRUE(unit->setLedConfiguration(lc));

                    EXPECT_TRUE(unit->readLedConfiguration(lc));
                    EXPECT_EQ(lc.irLed(), ir);
                    EXPECT_EQ(lc.redLed(), red);
                }
            }
        }

        {
            EXPECT_TRUE(unit->setMode(Mode::SPO2));
            for (auto&& ir : cc_table) {
                for (auto&& red : cc_table) {
                    auto s = m5::utility::formatString("HRONLY:IR:%u/RED:%u",
                                                       ir, red);
                    SCOPED_TRACE(s.c_str());
                    // M5_LOGI("%s", s.c_str());

                    LedConfiguration lc{};

                    EXPECT_TRUE(unit->setLedCurrent(ir, red));

                    EXPECT_TRUE(unit->readLedConfiguration(lc));
                    EXPECT_EQ(lc.irLed(), ir);
                    EXPECT_EQ(lc.redLed(), red);

                    lc.irLed(ir);
                    lc.redLed(red);
                    EXPECT_TRUE(unit->setLedConfiguration(lc));

                    EXPECT_TRUE(unit->readLedConfiguration(lc));
                    EXPECT_EQ(lc.irLed(), ir);
                    EXPECT_EQ(lc.redLed(), red);
                }
            }
        }
    }
}

TEST_P(TestMAX30100, Temperature) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->setMode(Mode::SPO2));
    EXPECT_TRUE(unit->disablePowerSave());
    SpO2Configuration sc{};
    sc.samplingRate(Sampling::Rate100);
    sc.ledPulseWidth(LedPulseWidth::PW1600);
    sc.highResolution(true);
    EXPECT_TRUE(unit->setSpO2Configuration(sc));
    EXPECT_TRUE(
        unit->setLedCurrent(CurrentControl::mA7_6, CurrentControl::mA7_6));

    for (auto&& m : mode_table) {
        auto s = m5::utility::formatString("Mode:%u", m);
        SCOPED_TRACE(s.c_str());
        EXPECT_TRUE(unit->setMode(m));

        TemperatureData td{};
        uint32_t cnt{8};
        while (cnt--) {
            EXPECT_TRUE(unit->measureTemperatureSingleshot(td));
            EXPECT_TRUE(std::isfinite(td.celsius()));
            EXPECT_TRUE(std::isfinite(td.fahrenheit()));
            // M5_LOGI("TempS>C:%f F:%f", td.celsius(), td.fahrenheit());
        }
    }
}

TEST_P(TestMAX30100, Reset) {
    EXPECT_TRUE(unit->setMode(Mode::SPO2));
    EXPECT_TRUE(unit->enablePowerSave());

    SpO2Configuration sc{};
    sc.samplingRate(Sampling::Rate100);
    sc.ledPulseWidth(LedPulseWidth::PW1600);
    sc.highResolution(true);
    EXPECT_TRUE(unit->setSpO2Configuration(sc));

    EXPECT_TRUE(
        unit->setLedCurrent(CurrentControl::mA7_6, CurrentControl::mA7_6));

    EXPECT_TRUE(unit->writeRegister8(FIFO_WRITE_POINTER, 1));
    EXPECT_TRUE(unit->writeRegister8(FIFO_READ_POINTER, 1));

    // reset
    EXPECT_TRUE(unit->reset());

    {
        ModeConfiguration mc{};
        SpO2Configuration sc{};
        LedConfiguration lc{};

        EXPECT_TRUE(unit->readModeConfiguration(mc));
        EXPECT_EQ(mc.value, 0);
        EXPECT_TRUE(unit->readSpO2Configuration(sc));
        EXPECT_EQ(sc.value, 0);
        EXPECT_TRUE(unit->readLedConfiguration(lc));
        EXPECT_EQ(lc.value, 0);

        uint8_t wptr{}, rptr{};
        EXPECT_TRUE(unit->readRegister8(FIFO_WRITE_POINTER, wptr, 0));
        EXPECT_TRUE(unit->readRegister8(FIFO_READ_POINTER, rptr, 0));
        EXPECT_EQ(wptr, 0U);
        EXPECT_EQ(rptr, 0U);
    }
}

TEST_P(TestMAX30100, Periodic) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->setMode(Mode::SPO2));
    EXPECT_TRUE(unit->disablePowerSave());
    SpO2Configuration sc{};
    sc.samplingRate(Sampling::Rate100);  // *1
    sc.ledPulseWidth(LedPulseWidth::PW1600);
    sc.highResolution(true);
    EXPECT_TRUE(unit->setSpO2Configuration(sc));

    EXPECT_TRUE(
        unit->setLedCurrent(CurrentControl::mA7_6, CurrentControl::mA7_6));

    auto start_at = m5::utility::millis();

    // wait first read (timeout 1sec)
    do {
        unit->update();
        m5::utility::delay(10);
    } while (!unit->updated() && m5::utility::millis() - start_at <= 1000);
    EXPECT_TRUE(unit->updated());

    EXPECT_FALSE(unit->full());
    EXPECT_FALSE(unit->empty());
    EXPECT_GT(unit->available(), 0U);

    while (unit->available()) {
        // M5_LOGI("IR:%u RED:%u", unit->ir(), unit->red());
        EXPECT_NE(unit->ir(), 0U);
        EXPECT_NE(unit->red(), 0U);
        EXPECT_EQ(unit->ir(), unit->oldest().ir());
        EXPECT_EQ(unit->red(), unit->oldest().red());
        unit->discard();
    }

    m5::utility::delay(100);  // Sampling about 10 times (not overflow)

    unit->update();
    EXPECT_TRUE(unit->updated());

    EXPECT_GE(unit->available(), 10U);
    auto retrived = unit->retrived();
    EXPECT_GT(retrived, 0U);
    EXPECT_FALSE(unit->full());
    EXPECT_FALSE(unit->empty());

    unit->flush();
    EXPECT_EQ(unit->available(), 0U);
    EXPECT_EQ(unit->retrived(), retrived);  // Not clear on flush
    EXPECT_FALSE(unit->full());
    EXPECT_TRUE(unit->empty());

    m5::utility::delay(200);  // Sampling about 20 times (overflow!)

    unit->update();
    EXPECT_TRUE(unit->updated());

    EXPECT_EQ(unit->available(), MAX_FIFO_DEPTH);
    EXPECT_GT(unit->retrived(), 0U);
    EXPECT_TRUE(unit->full());
    EXPECT_FALSE(unit->empty());
    EXPECT_GT(unit->overflow(), 0U);

    while (unit->available()) {
        // M5_LOGI("IR:%u RED:%u", unit->ir(), unit->red());
        EXPECT_NE(unit->ir(), 0U);
        EXPECT_NE(unit->red(), 0U);
        EXPECT_EQ(unit->ir(), unit->oldest().ir());
        EXPECT_EQ(unit->red(), unit->oldest().red());
        unit->discard();
    }
}
