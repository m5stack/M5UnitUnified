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

std::tuple<const char*, m5::unit::sht3x::Repeatability, bool> ss_table[] = {
    {"HighTrue", m5::unit::sht3x::Repeatability::High, true},
    {"MediumTrue", m5::unit::sht3x::Repeatability::Medium, true},
    {"LowTrue", m5::unit::sht3x::Repeatability::Low, true},
    {"HighFalse", m5::unit::sht3x::Repeatability::High, false},
    {"MediumFalse", m5::unit::sht3x::Repeatability::Medium, false},
    {"LowFalse", m5::unit::sht3x::Repeatability::Low, false},
};

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
        // Wire.begin(pin_num_sda, pin_num_scl, 400000U);

        ustr = m5::utility::formatString("%s:%s", unit.deviceName(),
                                         GetParam() ? "Bus" : "Wire");
        // printf("Test as %s\n", ustr.c_str());

        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override {
        // Wire.end();
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
    m5::unit::UnitSHT30 unit;
    std::string ustr{};
};

// true:Bus false:Wire
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestSHT3x,
//                         ::testing::Values(true, false));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestSHT3x, ::testing::Values(false));

TEST_P(TestSHT3x, SingleShot) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    // Cannot read periodic
    EXPECT_FALSE(unit.readMeasurement());

    for (auto&& e : ss_table) {
        const char* s{};
        m5::unit::sht3x::Repeatability rep;
        bool stretch{};
        std::tie(s, rep, stretch) = e;

        SCOPED_TRACE(s);

        int cnt{10};  // repeat 10 times
        while (cnt--) {
            EXPECT_TRUE(unit.measurementSingleShot(rep, stretch));
        }
    }
}

TEST_P(TestSHT3x, Periodic) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    constexpr std::tuple<const char*, m5::unit::sht3x::MPS,
                         m5::unit::sht3x::Repeatability>
        table[] = {
            {"HalfHigh", m5::unit::sht3x::MPS::MpsHalf,
             m5::unit::sht3x::Repeatability::High},
            {"HalfMedium", m5::unit::sht3x::MPS::MpsHalf,
             m5::unit::sht3x::Repeatability::Medium},
            {"HalfLow", m5::unit::sht3x::MPS::MpsHalf,
             m5::unit::sht3x::Repeatability::Low},
            {"1High", m5::unit::sht3x::MPS::Mps1,
             m5::unit::sht3x::Repeatability::High},
            {"1Medium", m5::unit::sht3x::MPS::Mps1,
             m5::unit::sht3x::Repeatability::Medium},
            {"1Low", m5::unit::sht3x::MPS::Mps1,
             m5::unit::sht3x::Repeatability::Low},
            {"2High", m5::unit::sht3x::MPS::Mps2,
             m5::unit::sht3x::Repeatability::High},
            {"2Medium", m5::unit::sht3x::MPS::Mps2,
             m5::unit::sht3x::Repeatability::Medium},
            {"2Low", m5::unit::sht3x::MPS::Mps2,
             m5::unit::sht3x::Repeatability::Low},
            {"10fHigh", m5::unit::sht3x::MPS::Mps10,
             m5::unit::sht3x::Repeatability::High},
            {"10Medium", m5::unit::sht3x::MPS::Mps10,
             m5::unit::sht3x::Repeatability::Medium},
            {"10Low", m5::unit::sht3x::MPS::Mps10,
             m5::unit::sht3x::Repeatability::Low},
        };
    constexpr std::chrono::milliseconds timeout_table[] = {
        std::chrono::milliseconds(2000 + 1000),
        std::chrono::milliseconds(1000 + 500),
        std::chrono::milliseconds(500 + 250),
        std::chrono::milliseconds(100 + 50),
    };

    {
        for (auto&& e : table) {
            const char* s{};
            m5::unit::sht3x::MPS mps;
            m5::unit::sht3x::Repeatability rep;
            std::tie(s, mps, rep) = e;
            SCOPED_TRACE(s);

            EXPECT_TRUE(unit.startPeriodicMeasurement(mps, rep));

            auto timeout  = timeout_table[m5::stl::to_underlying(mps)];
            auto start_at = std::chrono::steady_clock::now();
            do {
                Units.update();  // call readMeasurement in it
                m5::utility::delay(50);
            } while (!unit.updated() &&
                     (std::chrono::steady_clock::now() - start_at) <= timeout);
            EXPECT_TRUE(unit.updated());
            EXPECT_TRUE(unit.stopPeriodicMeasurement());
        }
    }

    // ART(4 mps)
    EXPECT_TRUE(unit.startPeriodicMeasurement(
        m5::unit::sht3x::MPS::MpsHalf, m5::unit::sht3x::Repeatability::High));
    EXPECT_TRUE(unit.accelerateResponseTime());

    auto timeout  = std::chrono::milliseconds(1000 / 4);
    auto start_at = std::chrono::steady_clock::now();
    do {
        Units.update();  // call readMeasurement in it
        m5::utility::delay(50);
    } while (!unit.updated() &&
             (std::chrono::steady_clock::now() - start_at) <= timeout);
    EXPECT_TRUE(unit.updated());

    // Cannot read all singleshot
    for (auto&& e : ss_table) {
        const char* s{};
        m5::unit::sht3x::Repeatability rep;
        bool stretch{};
        std::tie(s, rep, stretch) = e;

        SCOPED_TRACE(s);
        EXPECT_FALSE(unit.measurementSingleShot(rep, stretch));
    }
}

namespace {
#if 0
void printStatus(const m5::unit::sht3x::Status& s) {
    M5_LOGI("Status at begin: %u/%u/%u/%u/%u/%u/%u", s.alertPending(),
            s.heater(), s.trackingAlertRH(), s.trackingAlert(), s.reset(),
            s.command(), s.checksum());
}
#endif
}  // namespace

TEST_P(TestSHT3x, HeaterAndStatus) {
    m5::unit::sht3x::Status s{};

    EXPECT_TRUE(unit.startHeater());

    EXPECT_TRUE(unit.getStatus(s));
    // printStatus(s);
    EXPECT_TRUE(s.heater());

    // clearStatus will not clear heater status
    EXPECT_TRUE(unit.clearStatus());
    EXPECT_TRUE(unit.getStatus(s));
    // printStatus(s);
    EXPECT_TRUE(s.heater());

    EXPECT_TRUE(unit.stopHeater());
    EXPECT_TRUE(unit.getStatus(s));
    // printStatus(s);

    EXPECT_FALSE(s.heater());
}

TEST_P(TestSHT3x, Reset) {
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit.softReset());
    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    m5::unit::sht3x::Status s{};
    EXPECT_TRUE(unit.startHeater());

    EXPECT_TRUE(unit.softReset());

    EXPECT_TRUE(unit.getStatus(s));
    EXPECT_FALSE(s.alertPending());
    EXPECT_FALSE(s.heater());
    EXPECT_FALSE(s.trackingAlertRH());
    EXPECT_FALSE(s.trackingAlert());
    EXPECT_FALSE(s.reset());
    EXPECT_FALSE(s.command());
    EXPECT_FALSE(s.checksum());
}

TEST_P(TestSHT3x, Serial) {
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit.stopPeriodicMeasurement());

    {
        uint32_t sno{};
        char ssno[9]{};
        EXPECT_TRUE(unit.getSerialNumber(sno));
        EXPECT_TRUE(unit.getSerialNumber(ssno));

        // M5_LOGI("s:[%s] uint64:[%x]", ssno, sno);

        std::stringstream stream;
        stream << std::uppercase << std::setw(8) << std::hex << sno;
        std::string s(stream.str());
        EXPECT_STREQ(s.c_str(), ssno);
    }
}
