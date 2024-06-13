/*
  UnitTest for UnitSCD40/41

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <unit/unit_SCD4x.hpp>
#include <chrono>
#include <iostream>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::scd4x;

namespace {
// flot t uu int16 (temperature)
constexpr uint16_t float_to_uint16(const float f) {
    return f * 65536 / 175;
}
}  // namespace

// #define UNIT_TEST_SCD41

const ::testing::Environment* global_fixture =
    ::testing::AddGlobalTestEnvironment(new GlobalFixture<400000U>());

class TestSCD40 : public ComponentTestBase<UnitSCD40, bool> {
   protected:
    virtual UnitSCD40* get_instance() override {
#if defined(UNIT_TEST_SCD41)
        return new m5::unit::UnitSCD41();
#else
        return new m5::unit::UnitSCD40();
#endif
    }

    virtual bool is_using_hal() const override {
        return GetParam();
    };
};

class TestSCD41 : public ComponentTestBase<UnitSCD40, bool> {
   protected:
    virtual UnitSCD41* get_instance() override {
        return new m5::unit::UnitSCD41();
    }
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD40,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD40, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD40, ::testing::Values(false));

#if defined(UNIT_TEST_SCD41)
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD41,
//                          ::testing::Values(false, true));
//  INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD41, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD41, ::testing::Values(false));
#endif

TEST_P(TestSCD40, BasicCommand) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    EXPECT_TRUE(unit->startPeriodicMeasurement());
    // Return False if already started
    EXPECT_FALSE(unit->startPeriodicMeasurement());

    constexpr std::chrono::milliseconds TIMEOUT{5500};
    auto start_at = std::chrono::steady_clock::now();
    do {
        Units.update();  // call readMeasurement in it
        m5::utility::delay(500);
    } while (!unit->updated() &&
             (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
    EXPECT_TRUE(unit->updated());

    // These APIs result in an error during periodic detection
    {
        EXPECT_FALSE(unit->setTemperatureOffset(0));
        float offset{};
        EXPECT_FALSE(unit->getTemperatureOffset(offset));

        EXPECT_FALSE(unit->setSensorAltitude(0));
        uint16_t altitude{};
        EXPECT_FALSE(unit->getSensorAltitude(altitude));

        int16_t correction{};
        EXPECT_FALSE(unit->performForcedRecalibration(0, correction));

        EXPECT_FALSE(unit->setAutomaticSelfCalibrationEnabled(true));
        bool enabled{};
        EXPECT_FALSE(unit->getAutomaticSelfCalibrationEnabled(enabled));

        EXPECT_FALSE(unit->startLowPowerPeriodicMeasurement());

        EXPECT_FALSE(unit->persistSettings());

        uint64_t sno{};
        EXPECT_FALSE(unit->getSerialNumber(sno));

        bool malfunction{};
        EXPECT_FALSE(unit->performSelfTest(malfunction));

        EXPECT_FALSE(unit->performFactoryReset());

        EXPECT_FALSE(unit->reInit());
    }
    // These APIs can be used during periodic detection
    EXPECT_TRUE(unit->setAmbientPressure(0.0f));
}

TEST_P(TestSCD40, OnChipOutputSignalCompensation) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    {
        constexpr float OFFSET{1.234f};
        EXPECT_TRUE(unit->setTemperatureOffset(OFFSET));
        float offset{};
        EXPECT_TRUE(unit->getTemperatureOffset(offset));
        EXPECT_EQ(float_to_uint16(offset), float_to_uint16(OFFSET))
            << "offset:" << offset << " OFFSET:" << OFFSET;
    }

    {
        constexpr uint16_t ALTITUDE{3776};
        EXPECT_TRUE(unit->setSensorAltitude(ALTITUDE));
        uint16_t altitude{};
        EXPECT_TRUE(unit->getSensorAltitude(altitude));
        EXPECT_EQ(altitude, ALTITUDE);
    }
}

TEST_P(TestSCD40, FieldCalibration) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    {
        int16_t correction{};
        EXPECT_TRUE(unit->performForcedRecalibration(1234, correction));
    }

    {
        EXPECT_TRUE(unit->setAutomaticSelfCalibrationEnabled(false));
        bool enabled{};
        EXPECT_TRUE(unit->getAutomaticSelfCalibrationEnabled(enabled));
        EXPECT_FALSE(enabled);

        EXPECT_TRUE(unit->setAutomaticSelfCalibrationEnabled(true));
        EXPECT_TRUE(unit->getAutomaticSelfCalibrationEnabled(enabled));
        EXPECT_TRUE(enabled);
    }
}

TEST_P(TestSCD40, LowPowerOperation) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    EXPECT_TRUE(unit->startLowPowerPeriodicMeasurement());
    // Return False if already started
    EXPECT_FALSE(unit->startLowPowerPeriodicMeasurement());

    constexpr std::chrono::milliseconds TIMEOUT{30500};
    auto start_at = std::chrono::steady_clock::now();
    do {
        Units.update();  // call readMeasurement in it
        m5::utility::delay(500);
    } while (!unit->updated() &&
             (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
    EXPECT_TRUE(unit->updated());
}

TEST_P(TestSCD40, AdvancedFeatures) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    {
        // Read direct [MSB] SNB_3, SNB_2, CRC, SNB_1, SNB_0, CRC [LSB]
        std::array<uint8_t, 9> rbuf{};
        EXPECT_TRUE(
            unit->readRegister(m5::unit::scd4x::command::GET_SERIAL_NUMBER,
                               rbuf.data(), rbuf.size(), 1));

        // M5_LOGI("%02x%02x%02x%02x%02x%02x", rbuf[0], rbuf[1], rbuf[3],
        // rbuf[4],
        //          rbuf[6], rbuf[7]);

        m5::types::big_uint16_t w0(rbuf[0], rbuf[1]);
        m5::types::big_uint16_t w1(rbuf[3], rbuf[4]);
        m5::types::big_uint16_t w2(rbuf[6], rbuf[7]);
        uint64_t d_sno = (((uint64_t)w0.get()) << 32) |
                         (((uint64_t)w1.get()) << 16) | ((uint64_t)w2.get());

        // M5_LOGI("d_sno[%llX]", d_sno);

        //
        uint64_t sno{};
        char ssno[13]{};
        EXPECT_TRUE(unit->getSerialNumber(sno));
        EXPECT_TRUE(unit->getSerialNumber(ssno));

        // M5_LOGI("s:[%s] uint64:[%x]", ssno, sno);

        EXPECT_EQ(sno, d_sno);

        std::stringstream stream;
        stream << std::uppercase << std::setw(12) << std::hex << sno;
        std::string s(stream.str());
        EXPECT_STREQ(s.c_str(), ssno);
    }

    // Set
    constexpr float OFFSET{1.234f};
    EXPECT_TRUE(unit->setTemperatureOffset(OFFSET));
    constexpr uint16_t ALTITUDE{3776};
    EXPECT_TRUE(unit->setSensorAltitude(ALTITUDE));
    EXPECT_TRUE(unit->setAutomaticSelfCalibrationEnabled(false));

    EXPECT_TRUE(unit->persistSettings());  // Save EEPROM

    // Overwrite settings
    EXPECT_TRUE(unit->setTemperatureOffset(OFFSET * 2));
    EXPECT_TRUE(unit->setSensorAltitude(ALTITUDE * 2));
    EXPECT_TRUE(unit->setAutomaticSelfCalibrationEnabled(true));
    EXPECT_EQ(float_to_uint16(unit->getTemperatureOffset()),
              float_to_uint16(OFFSET * 2));
    EXPECT_EQ(unit->getSensorAltitude(), ALTITUDE * 2);
    EXPECT_TRUE(unit->getAutomaticSelfCalibrationEnabled());

    EXPECT_TRUE(unit->reInit());  // Load EEPROM

    // Check saved settings
    EXPECT_EQ(float_to_uint16(unit->getTemperatureOffset()),
              float_to_uint16(OFFSET));
    EXPECT_EQ(unit->getSensorAltitude(), ALTITUDE);
    EXPECT_FALSE(unit->getAutomaticSelfCalibrationEnabled());

    bool malfunction{};
    EXPECT_TRUE(unit->performSelfTest(malfunction));

    EXPECT_TRUE(unit->performFactoryReset());  // Reset EEPROM

    EXPECT_NE(float_to_uint16(unit->getTemperatureOffset()),
              float_to_uint16(OFFSET));
    EXPECT_NE(unit->getSensorAltitude(), ALTITUDE);
    EXPECT_TRUE(unit->getAutomaticSelfCalibrationEnabled());
}

#if defined(UNIT_TEST_SCD41)
TEST_P(TestSCD41, LowPowerSingleShot) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unitSCD41.stopPeriodicMeasurement());

    {
        EXPECT_TRUE(unitSCD41.measureSingleShot());
        constexpr std::chrono::milliseconds TIMEOUT{5500};
        auto start_at = std::chrono::steady_clock::now();
        do {
            Units.update();  // call readMeasurement in it
            m5::utility::delay(500);
        } while (!unitSCD41.updated() &&
                 (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
        EXPECT_TRUE(unitSCD41.updated());
    }

    {
        EXPECT_TRUE(unitSCD41.measureSingleShotRHTOnly());
        constexpr std::chrono::milliseconds TIMEOUT{100};
        auto start_at = std::chrono::steady_clock::now();
        do {
            Units.update();  // call readMeasurement in it
            m5::utility::delay(50);
        } while (!unitSCD41.updated() &&
                 (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
        EXPECT_TRUE(unitSCD41.updated());
        EXPECT_EQ(unitSCD41.co2(), 0);  // RHT only
        EXPECT_GE(unitSCD41.temperature(), -10.f);
        EXPECT_GE(unitSCD41.humidity(), 0.f);
    }
}
#endif
