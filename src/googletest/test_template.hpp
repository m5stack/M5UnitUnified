/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file test_template.hpp
  @brief Helper for testing UnitComponent
  @note Depends on GoogleTest
*/
#ifndef M5_UNIT_COMPONENT_GOOGLETEST_TEMPLATE_HPP
#define M5_UNIT_COMPONENT_GOOGLETEST_TEMPLATE_HPP

#include "../M5UnitComponent.hpp"
#include <type_traits>
#include <Wire.h>
#include <esp32-hal-i2c.h>

namespace m5 {
namespace unit {
/*!
  @namespace googletest
  @brief For GoogleTest
 */
namespace googletest {

/*!
  @class GlobalFixture
  @brief Overall test environment configuration
  @tparam FREQ TwoWire operating frequency
  @tparam WNUM TwoWire number to be used (0 as default)
 */
template <uint32_t FREQ, uint32_t WNUM = 0>
class GlobalFixture : public ::testing::Environment {
    static_assert(WNUM < 2, "Wire number must be lesser than 2");

public:
    void SetUp() override
    {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

#if defined(CONFIG_IDF_TARGET_ESP32C6)
        TwoWire* w[1] = {&Wire};
#else
        TwoWire* w[2] = {&Wire, &Wire1};
#endif
        if (WNUM < m5::stl::size(w) && i2cIsInit(WNUM)) {
            M5_LOGW("Already inititlized Wire %d. Terminate and restart FREQ %u", WNUM, FREQ);
            w[WNUM]->end();
        }
        w[WNUM]->begin(pin_num_sda, pin_num_scl, FREQ);
    }
};

/*!
  @class ComponentTestBase
  @brief UnitComponent Derived class for testing (I2C)
  @tparam U m5::unit::Component-derived classes to be tested
  @tparam TP parameter type for testing. see also INSTANTIATE_TEST_SUITE_P
 */
template <typename U, typename TP>
class ComponentTestBase : public ::testing::TestWithParam<TP> {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            GTEST_SKIP();
            return;
        }
        ustr = m5::utility::formatString("%s:%s", unit->deviceName(), is_using_hal() ? "HAL" : "Wire");
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override
    {
    }

    virtual bool begin()
    {
        if (is_using_hal()) {
            // Using M5HAL
            auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
            auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
            m5::hal::bus::I2CBusConfig i2c_cfg;
            i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
            i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
            auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
            return Units.add(*unit, i2c_bus ? i2c_bus.value() : nullptr) && Units.begin();
        }
        // Using TwoWire
        return Units.add(*unit, Wire) && Units.begin();
    }

    //!@brief Function returning true if M5HAL is used (decision based on TP)
    virtual bool is_using_hal() const = 0;
    //! @brief return m5::unit::Component-derived class instance (decision based on TP)
    virtual U* get_instance() = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

/*!
  @class GPIOComponentTestBase
  @brief UnitComponent Derived class for testing (GPIO)
  @tparam U m5::unit::Component-derived classes to be tested
  @tparam TP parameter type for testing. see also INSTANTIATE_TEST_SUITE_P
 */
template <typename U, typename TP>
class GPIOComponentTestBase : public ::testing::TestWithParam<TP> {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            GTEST_SKIP();
            return;
        }
        ustr = m5::utility::formatString("%s:%s", unit->deviceName(), is_using_hal() ? "HAL" : "GPIO");
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
            GTEST_SKIP();
        }
    }

    virtual void TearDown() override
    {
    }

    virtual bool begin()
    {
        auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
        auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
        if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
            M5_LOGW("PortB is not available");
            Wire.end();
            pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
            pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
        }
        M5_LOGI("getPin: %d,%d", pin_num_gpio_in, pin_num_gpio_out);

        if (is_using_hal()) {
            // Using M5HAL
            // TODO Not yet
            return false;
        }
        // Using TwoWire
        return Units.add(*unit, pin_num_gpio_in, pin_num_gpio_out) && Units.begin();
    }

    //!@brief Function returning true if M5HAL is used (decision based on TP)
    virtual bool is_using_hal() const = 0;
    //! @brief return m5::unit::Component-derived class instance (decision based on TP)
    virtual U* get_instance() = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
