/*!
  UnitTest for UnitSHT3x

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_SHT3x.hpp>
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
class TestSHT3x : public ::testing::TestWithParam<bool> {
   protected:
    virtual void SetUp() override {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
        // printf("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
        Wire.begin(pin_num_sda, pin_num_scl, 400000U);

#if 0
        // SCD40/41 are same default address
        Wire.beginTransmission(m5::unit::UnitSCD40::DEFAULT_ADDRESS);
        m5::utility::delay(1);
        if (Wire.endTransmission() != 0) {
            FAIL() << "SCD4x was not connected";
            GTEST_SKIP();
        }
#endif
        ustr = m5::utility::formatString("%s:%s", unit.deviceName(),
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
    m5::unit::UnitSHT3x unit;
    std::string ustr{};
};

// true:Bus false:Wire
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSHT3x,
//                         ::testing::Values(true, false));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSHT3x, ::testing::Values(false));

TEST_P(TestSHT3x, SingleShot) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    std::tuple<const char*, m5::unit::UnitSHT3x::Repeatability, bool> table[] =
        {
            {"HighTrue", m5::unit::UnitSHT3x::Repeatability::High, true},
            {"MediumTrue", m5::unit::UnitSHT3x::Repeatability::Medium, true},
            {"LowTrue", m5::unit::UnitSHT3x::Repeatability::Low, true},
            {"HighFalse", m5::unit::UnitSHT3x::Repeatability::High, false},
            {"MediumFalse", m5::unit::UnitSHT3x::Repeatability::Medium, false},
            {"LowFalse", m5::unit::UnitSHT3x::Repeatability::Low, false},
        };

    for (auto&& e : table) {
        const char* s{};
        m5::unit::UnitSHT3x::Repeatability rep;
        bool stretch{};
        std::tie(s, rep, stretch) = e;
        EXPECT_TRUE(unit.measurementSingle(rep, stretch)) << s;
    }
}

#if 0
TEST_P(TestSHT3x, BasicCommand) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    EXPECT_TRUE(unit.startPeriodicMeasurement());
    // Return False if already started
    EXPECT_FALSE(unit.startPeriodicMeasurement());

    constexpr std::chrono::milliseconds TIMEOUT{5500};
    auto start_at = std::chrono::steady_clock::now();
    do {
        Units.update();  // call readMeasurement in it
        m5::utility::delay(500);
    } while (!unit.updated() &&
             (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
    EXPECT_TRUE(unit.updated());

    // These APIs result in an error during periodic detection
    {
        EXPECT_FALSE(unit.setTemperatureOffset(0));
        float offset{};
        EXPECT_FALSE(unit.getTemperatureOffset(offset));

        EXPECT_FALSE(unit.setSensorAltitude(0));
        uint16_t altitude{};
        EXPECT_FALSE(unit.getSensorAltitude(altitude));

        int16_t correction{};
        EXPECT_FALSE(unit.performForcedRecalibration(0, correction));

        EXPECT_FALSE(unit.setAutomaticSelfCalibrationEnabled(true));
        bool enabled{};
        EXPECT_FALSE(unit.getAutomaticSelfCalibrationEnabled(enabled));

        EXPECT_FALSE(unit.startLowPowerPeriodicMeasurement());

        EXPECT_FALSE(unit.persistSettings());

        uint64_t sno{};
        EXPECT_FALSE(unit.getSerialNumber(sno));

        bool malfunction{};
        EXPECT_FALSE(unit.performSelfTest(malfunction));

        EXPECT_FALSE(unit.performFactoryReset());

        EXPECT_FALSE(unit.reInit());
    }
    // These APIs can be used during periodic detection
    EXPECT_TRUE(unit.setAmbientPressure(0.0f));
}

TEST_P(TestSHT3x, OnChipOutputSignalCompensation) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    {
        constexpr float OFFSET{1.234f};
        EXPECT_TRUE(unit.setTemperatureOffset(OFFSET));
        float offset{};
        EXPECT_TRUE(unit.getTemperatureOffset(offset));
        EXPECT_EQ(float_to_uint16(offset), float_to_uint16(OFFSET))
            << "offset:" << offset << " OFFSET:" << OFFSET;
    }

    {
        constexpr uint16_t ALTITUDE{3776};
        EXPECT_TRUE(unit.setSensorAltitude(ALTITUDE));
        uint16_t altitude{};
        EXPECT_TRUE(unit.getSensorAltitude(altitude));
        EXPECT_EQ(altitude, ALTITUDE);
    }
}

TEST_P(TestSHT3x, FieldCalibration) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    {
        int16_t correction{};
        EXPECT_TRUE(unit.performForcedRecalibration(1234, correction));
    }

    {
        EXPECT_TRUE(unit.setAutomaticSelfCalibrationEnabled(false));
        bool enabled{};
        EXPECT_TRUE(unit.getAutomaticSelfCalibrationEnabled(enabled));
        EXPECT_FALSE(enabled);

        EXPECT_TRUE(unit.setAutomaticSelfCalibrationEnabled(true));
        EXPECT_TRUE(unit.getAutomaticSelfCalibrationEnabled(enabled));
        EXPECT_TRUE(enabled);
    }
}

TEST_P(TestSHT3x, LowPowerOperation) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    EXPECT_TRUE(unit.startLowPowerPeriodicMeasurement());
    // Return False if already started
    EXPECT_FALSE(unit.startLowPowerPeriodicMeasurement());

    constexpr std::chrono::milliseconds TIMEOUT{30500};
    auto start_at = std::chrono::steady_clock::now();
    do {
        Units.update();  // call readMeasurement in it
        m5::utility::delay(500);
    } while (!unit.updated() &&
             (std::chrono::steady_clock::now() - start_at) <= TIMEOUT);
    EXPECT_TRUE(unit.updated());
}

TEST_P(TestSHT3x, AdvancedFeatures) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    {
        uint64_t sno{};
        char ssno[13]{};
        EXPECT_TRUE(unit.getSerialNumber(sno));
        EXPECT_TRUE(unit.getSerialNumber(ssno));

        std::stringstream stream;
        stream << std::uppercase << std::setw(12) << std::hex << sno;
        std::string s(stream.str());
        EXPECT_STREQ(s.c_str(), ssno);
    }

    // Set
    constexpr float OFFSET{1.234f};
    EXPECT_TRUE(unit.setTemperatureOffset(OFFSET));
    constexpr uint16_t ALTITUDE{3776};
    EXPECT_TRUE(unit.setSensorAltitude(ALTITUDE));
    EXPECT_TRUE(unit.setAutomaticSelfCalibrationEnabled(false));

    EXPECT_TRUE(unit.persistSettings());  // Save EEPROM

    // Overwrite settings
    EXPECT_TRUE(unit.setTemperatureOffset(OFFSET * 2));
    EXPECT_TRUE(unit.setSensorAltitude(ALTITUDE * 2));
    EXPECT_TRUE(unit.setAutomaticSelfCalibrationEnabled(true));
    EXPECT_EQ(float_to_uint16(unit.getTemperatureOffset()),
              float_to_uint16(OFFSET * 2));
    EXPECT_EQ(unit.getSensorAltitude(), ALTITUDE * 2);
    EXPECT_TRUE(unit.getAutomaticSelfCalibrationEnabled());

    EXPECT_TRUE(unit.reInit());  // Load EEPROM

    // Check saved settings
    EXPECT_EQ(float_to_uint16(unit.getTemperatureOffset()),
              float_to_uint16(OFFSET));
    EXPECT_EQ(unit.getSensorAltitude(), ALTITUDE);
    EXPECT_FALSE(unit.getAutomaticSelfCalibrationEnabled());

    bool malfunction{};
    EXPECT_TRUE(unit.performSelfTest(malfunction));

    EXPECT_TRUE(unit.performFactoryReset());  // Reset EEPROM

    EXPECT_NE(float_to_uint16(unit.getTemperatureOffset()),
              float_to_uint16(OFFSET));
    EXPECT_NE(unit.getSensorAltitude(), ALTITUDE);
    EXPECT_TRUE(unit.getAutomaticSelfCalibrationEnabled());
}

#endif
