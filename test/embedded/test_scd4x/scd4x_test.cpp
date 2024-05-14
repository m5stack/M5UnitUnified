/*!
  UnitTest for UnitSCD40/41

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_SCD4x.hpp>
#include <chrono>
#include <iostream>

namespace {
// flot t uu int16 (temperature)
constexpr uint16_t float_to_uint16(const float f) {
    return f * 65536 / 175;
}

}  // namespace

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

// bool true: Using bus false: using wire
class TestSCD4x : public ::testing::TestWithParam<bool> {
   protected:
    virtual void SetUp() override {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
        // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
        Wire.begin(pin_num_sda, pin_num_scl, 400000U);

        unit = get_instance();
        if (!unit) {
            GTEST_SKIP() << "Skip as there are no instances";
        }

        ustr = m5::utility::formatString("%s:%s", unit->deviceName(),
                                         GetParam() ? "Bus" : "Wire");
        // printf("Test as %s\n", ustr.c_str());

        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override {
        Wire.end();
    }

    virtual m5::unit::UnitSCD40* get_instance() {
        return nullptr;
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

            return Units.add(*unit, i2c_bus ? i2c_bus.value() : nullptr) &&
                   Units.begin();
        }
        // Wire
        return Units.add(*unit, Wire) && Units.begin();
    }

    m5::unit::UnitUnified Units;
    m5::unit::UnitSCD40* unit{};  // SCD40 or SCD41
    std::string ustr{};
};

// #define UNIT_TEST_SCD41

class TestSCD40 : public TestSCD4x {
   protected:
    virtual m5::unit::UnitSCD40* get_instance() override {
        return &unitSCD4x;
    }
#if 0
    virtual bool begin() override {
        if (GetParam()) {
            // Bus
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

            m5::hal::bus::I2CBusConfig i2c_cfg;
            i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
            i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
            auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

            return Units.add(unitSCD4x, i2c_bus ? i2c_bus.value() : nullptr) &&
                   Units.begin();
        }
        // Wire
        return Units.add(unitSCD4x, Wire) && Units.begin();
    }
#endif

#if defined(UNIT_TEST_SCD41)
    // TEST as SCD41
    m5::unit::UnitSCD41 unitSCD4x;
#else
    // TEST as SCD40
    m5::unit::UnitSCD40 unitSCD4x;
#endif
};

class TestSCD41 : public TestSCD4x {
   protected:
    virtual m5::unit::UnitSCD40* get_instance() override {
        return &unitSCD41;
    }

    m5::unit::UnitSCD41 unitSCD41;
};

INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD40,
                         ::testing::Values(true, false));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD40, ::testing::Values(true));

#if defined(UNIT_TEST_SCD41)
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD41,
                         ::testing::Values(true, false));

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSCD41, ::testing::Values(true));
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
        uint64_t sno{};
        char ssno[13]{};
        EXPECT_TRUE(unit->getSerialNumber(sno));
        EXPECT_TRUE(unit->getSerialNumber(ssno));

        //        M5_LOGI("s:[%s] uint64:[%x]", ssno, sno);

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
