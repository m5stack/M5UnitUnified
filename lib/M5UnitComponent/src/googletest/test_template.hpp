/*
  UnitCOmponent test template for googletest

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_COMPONENT_GOOGLETEST_TEMPLATE_HPP
#define M5_UNIT_COMPONENT_GOOGLETEST_TEMPLATE_HPP

#include "../M5UnitComponent.hpp"
#include <type_traits>
#include <Wire.h>
#include <esp32-hal-i2c.h>

namespace m5 {
namespace unit {
namespace googletest {

template <uint32_t FREQ, uint32_t WNUM = 0>
class GlobalFixture : public ::testing::Environment {
    static_assert(WNUM < 2, "Wire number must be lesser than 2");

   public:
    void SetUp() override {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

        if (i2cIsInit(WNUM)) {
            M5_LOGW("Already inititlized Wire");
            Wire.end();
        }
        TwoWire* w[2] = {&Wire, &Wire1};
        w[WNUM]->begin(pin_num_sda, pin_num_scl, FREQ);
    }
};

template <typename U, typename TP>
class ComponentTestBase : public ::testing::TestWithParam<TP> {
    static_assert(std::is_base_of<m5::unit::Component, U>::value,
                  "U must be derived from Component");

   protected:
    virtual void SetUp() override {
        if (!is_using_hal()) {
            // auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            // auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            //  M5_LOGI("getPin: SDA:%u SCL:%u\n", pin_num_sda, pin_num_scl);
            //  Wire.begin(pin_num_sda, pin_num_scl, 400000U);
        }

        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            GTEST_SKIP();
            return;
        }

        ustr = m5::utility::formatString("%s:%s", unit->deviceName(),
                                         is_using_hal() ? "HAL" : "Wire");
        // M5_LOGI("Test as %s\n", ustr.c_str());

        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override {
        if (!is_using_hal()) {
            // Wire.end();
        }
    }

    virtual bool begin() {
        if (is_using_hal()) {
            // HAL
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            m5::hal::bus::I2CBusConfig i2c_cfg;
            i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
            i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
            auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
            return Units.add(*unit, i2c_bus ? i2c_bus.value() : nullptr) &&
                   Units.begin();
        }
        // TwoWire
        return Units.add(*unit, Wire) && Units.begin();
    }

    virtual bool is_using_hal() const = 0;
    virtual U* get_instance()         = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
