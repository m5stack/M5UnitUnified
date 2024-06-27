/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for KMeterISO
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_KmeterISO.hpp>
#include <chrono>
#include <thread>
#include <iostream>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::kmeter;

namespace {
float celsiusToFahrenheit(float celsius) {
    return (celsius * 9.f / 5.f) + 32.f;
}

}  // namespace

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<100000U>());

class TestKmeterISO : public ComponentTestBase<UnitKmeterISO, bool> {
   protected:
    virtual UnitKmeterISO* get_instance() override {
        return new m5::unit::UnitKmeterISO();
    }
    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestKmeterISO,
//                         ::testing::Values(false, true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestKmeterISO,
// ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestKmeterISO, ::testing::Values(false));

TEST_P(TestKmeterISO, Version) {
    SCOPED_TRACE(ustr);

    uint8_t ver{0x00};
    EXPECT_TRUE(unit->readFirmwareVersion(ver));
    EXPECT_NE(ver, 0x00);
}

TEST_P(TestKmeterISO, Measurement) {
    SCOPED_TRACE(ustr);

    uint8_t status{0xFF};
    EXPECT_TRUE(unit->readStatus(status));
    EXPECT_EQ(status, 0);

    int32_t c{}, f{};
    float cc{}, ff{};
    char sc[9]{}, sf[9]{};

    {
        EXPECT_TRUE(unit->readCelsiusTemperature(c));
        EXPECT_TRUE(unit->readFahrenheitTemperature(f));
        cc = UnitKmeterISO::conversion(c);
        ff = UnitKmeterISO::conversion(f);
        EXPECT_NEAR(celsiusToFahrenheit(cc), ff, 0.01f);

        EXPECT_TRUE(unit->readCelsiusTemperatureString(sc));
        EXPECT_TRUE(unit->readFahrenheitTemperatureString(sf));
        // M5_LOGW("%f,[%s] %f,[%s]", cc, sc, ff, sf);
    }

    {
        EXPECT_TRUE(unit->readInternalCelsiusTemperature(c));
        EXPECT_TRUE(unit->readInternalFahrenheitTemperature(f));
        cc = UnitKmeterISO::conversion(c);
        ff = UnitKmeterISO::conversion(f);
        EXPECT_NEAR(celsiusToFahrenheit(cc), ff, 0.01f);

        EXPECT_TRUE(unit->readInternalCelsiusTemperatureString(sc));
        EXPECT_TRUE(unit->readInternalFahrenheitTemperatureString(sf));
        // M5_LOGW("%f,[%s] %f,[%s]", cc, sc, ff, sf);
    }
}

TEST_P(TestKmeterISO, Periodic) {
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    constexpr uint32_t interval{2500};
    EXPECT_TRUE(unit->startPeriodicMeasurement(interval));
    EXPECT_TRUE(unit->inPeriodic());

    auto now        = m5::utility::millis();
    auto timeout_at = now + interval;
    uint8_t count{0};
    // Between first and second mesured
    do {
        unit->update();
        bool upd = unit->updated();
        count += upd ? 1 : 0;
        now = m5::utility::millis();
        if (upd && count == 1) {  // First?
            timeout_at = now + interval;
        }
        std::this_thread::yield();
    } while (count < 2 && now <= timeout_at);
    EXPECT_EQ(count, 2);
    EXPECT_LE(now, timeout_at);
}

TEST_P(TestKmeterISO, I2CAddress) {
    SCOPED_TRACE(ustr);

    uint8_t ver{}, addr{};
    EXPECT_FALSE(unit->changeI2CAddress(0x07));
    EXPECT_FALSE(unit->changeI2CAddress(0x78));

    EXPECT_TRUE(unit->changeI2CAddress(0x08));

    EXPECT_TRUE(unit->readI2CAddress(addr));
    EXPECT_EQ(addr, 0x08);
    EXPECT_TRUE(unit->readFirmwareVersion(ver));
    EXPECT_NE(ver, 0x00);

    EXPECT_TRUE(unit->changeI2CAddress(0x77));
    EXPECT_TRUE(unit->readFirmwareVersion(ver));
    EXPECT_NE(ver, 0x00);

    EXPECT_TRUE(unit->changeI2CAddress(0x52));
    EXPECT_TRUE(unit->readFirmwareVersion(ver));
    EXPECT_NE(ver, 0x00);

    EXPECT_TRUE(unit->changeI2CAddress(UnitKmeterISO::DEFAULT_ADDRESS));
    EXPECT_TRUE(unit->readFirmwareVersion(ver));
    EXPECT_NE(ver, 0x00);
}
