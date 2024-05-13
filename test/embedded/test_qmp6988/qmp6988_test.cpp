/*!
  UnitTest for UnitQMP6988

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/

// Move to each libarry

#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <unit/unit_QMP6988.hpp>
#include <chrono>

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
class TestQMP6988 : public ::testing::TestWithParam<bool> {
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
    m5::unit::UnitQMP6988 unit{m5::unit::UnitQMP6988::DEFAULT_SLAVE_ADDRESS};
    std::string ustr{};
};

// true:Bus false:Wire
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestQMP6988,
//                         ::testing::Values(true, false));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestQMP6988, ::testing::Values(false));

TEST_P(TestQMP6988, MeasurementCondition) {
    SCOPED_TRACE(ustr);

    m5::unit::qmp6988::Average t;
    m5::unit::qmp6988::Average p;
    m5::unit::qmp6988::PowerMode m;
    EXPECT_TRUE(unit.getMeasurementCondition(t, p, m));

    M5_LOGI("%x/%x/%x", (uint8_t)t, (uint8_t)p, (uint8_t)m);

    constexpr std::tuple<const char*, m5::unit::qmp6988::Average,
                         m5::unit::qmp6988::Average,
                         m5::unit::qmp6988::PowerMode>
        table[] = {
            {"SkipSkipSleep", m5::unit::qmp6988::Average::Skip,
             m5::unit::qmp6988::Average::Skip,
             m5::unit::qmp6988::PowerMode::Sleep},
            {"14Force", m5::unit::qmp6988::Average::Avg1,
             m5::unit::qmp6988::Average::Avg4,
             m5::unit::qmp6988::PowerMode::Force},
            {"648Normal", m5::unit::qmp6988::Average::Avg64,
             m5::unit::qmp6988::Average::Avg8,
             m5::unit::qmp6988::PowerMode::Normal},
            {"6464Normal", m5::unit::qmp6988::Average::Avg64,
             m5::unit::qmp6988::Average::Avg64,
             m5::unit::qmp6988::PowerMode::Normal},
        };

    for (auto&& e : table) {
        const char* s{};
        m5::unit::qmp6988::Average ta;
        m5::unit::qmp6988::Average pa;
        m5::unit::qmp6988::PowerMode mode;
        std::tie(s, ta, pa, mode) = e;

        SCOPED_TRACE(s);

        EXPECT_TRUE(unit.setMeasurementCondition(ta, pa, mode));
        EXPECT_TRUE(unit.getMeasurementCondition(t, p, m));

        M5_LOGI("%x/%x/%x", (uint8_t)t, (uint8_t)p, (uint8_t)m);

        EXPECT_EQ(t, ta);
        EXPECT_EQ(p, pa);
        EXPECT_EQ(m, mode);
    }
}
