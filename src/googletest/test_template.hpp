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
#include <SPI.h>
#include <esp32-hal-i2c.h>
#include <M5Unified.h>

namespace m5 {
namespace unit {
/*!
  @namespace googletest
  @brief For GoogleTest
 */
namespace googletest {

/*!
  @class I2CComponentTestBase
  @brief UnitComponent derived class for testing (I2C)
  @tparam U m5::unit::Component-derived classes to be tested
  @details Provides board-aware begin() with 3-branch logic:
  - NessoN1: SoftwareI2C via M5HAL (GROVE on port_b GPIO 5/4)
  - NanoC6: M5.Ex_I2C (avoids Wire/Ex_I2C dual-driver conflict)
  - Others: Wire (initialized internally via begin_with_wire())
 */
template <typename U>
class I2CComponentTestBase : public ::testing::Test {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            return;
        }
        ustr = m5::utility::formatString("%s", unit->deviceName());
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
        }
    }

    virtual void TearDown() override
    {
    }

    virtual bool begin()
    {
        auto board = M5.getBoard();
        if (board == m5::board_t::board_ArduinoNessoN1) {
            // NessoN1: GROVE is port_b (GPIO 5/4). SoftwareI2C via M5HAL
            auto sda = M5.getPin(m5::pin_name_t::port_b_out);
            auto scl = M5.getPin(m5::pin_name_t::port_b_in);
            return begin_with_software_i2c(sda, scl);
        }
        if (board == m5::board_t::board_M5NanoC6) {
            // NanoC6: Use Ex_I2C (avoids Wire/Ex_I2C dual-driver conflict)
            return begin_with_ex_i2c();
        }
        // Standard boards: Wire (initialized here with unit's clock)
        return begin_with_wire(Wire);
    }

    bool begin_with_wire(TwoWire& wire, uint32_t wnum = 0)
    {
        auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
        auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
        auto freq        = unit->component_config().clock;
        if (i2cIsInit(wnum)) {
            M5_LOGD("Already initialized Wire%u. Terminate and restart FREQ %u", wnum, freq);
            wire.end();
        }
        M5_LOGI("Wire begin SDA:%u SCL:%u FREQ:%u", pin_num_sda, pin_num_scl, freq);
        wire.begin(pin_num_sda, pin_num_scl, freq);
        return Units.add(*unit, wire) && Units.begin();
    }

    bool begin_with_ex_i2c()
    {
        M5_LOGI("Using Ex_I2C");
        return Units.add(*unit, M5.Ex_I2C) && Units.begin();
    }

    bool begin_with_software_i2c(int8_t sda, int8_t scl)
    {
        m5::hal::bus::I2CBusConfig i2c_cfg;
        i2c_cfg.pin_sda = m5::hal::gpio::getPin(sda);
        i2c_cfg.pin_scl = m5::hal::gpio::getPin(scl);
        auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
        M5_LOGI("Using M5HAL SDA:%u SCL:%u", sda, scl);
        return Units.add(*unit, i2c_bus ? i2c_bus.value() : nullptr) && Units.begin();
    }

    //! @brief return m5::unit::Component-derived class instance
    virtual U* get_instance() = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

/*!
  @class GPIOComponentTestBase
  @brief UnitComponent derived class for testing (GPIO)
  @tparam U m5::unit::Component-derived classes to be tested
 */
template <typename U>
class GPIOComponentTestBase : public ::testing::Test {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            return;
        }
        ustr = m5::utility::formatString("%s:GPIO", unit->deviceName());
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
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
        return Units.add(*unit, pin_num_gpio_in, pin_num_gpio_out) && Units.begin();
    }

    //! @brief return m5::unit::Component-derived class instance
    virtual U* get_instance() = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

/*!
  @class UARTComponentTestBase
  @brief UnitComponent derived class for testing (UART)
  @tparam U m5::unit::Component-derived classes to be tested
 */
template <typename U>
class UARTComponentTestBase : public ::testing::Test {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            return;
        }
        ustr = m5::utility::formatString("%s:UART", unit->deviceName());
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
        }
    }

    virtual void TearDown() override
    {
    }

    virtual bool begin()
    {
        serial = init_serial();
        return serial && Units.add(*unit, *serial) && Units.begin();
    }

    //! @brief return m5::unit::Component-derived class instance
    virtual U* get_instance() = 0;
    //! @brief Initialize the serial to be used
    virtual HardwareSerial* init_serial() = 0;

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
    HardwareSerial* serial{};
};

/*!
  @class SPIComponentTestBase
  @brief UnitComponent derived class for testing (SPI)
  @tparam U m5::unit::Component-derived classes to be tested
 */
template <typename U>
class SPIComponentTestBase : public ::testing::Test {
    static_assert(std::is_base_of<m5::unit::Component, U>::value, "U must be derived from Component");

protected:
    virtual void SetUp() override
    {
        unit.reset(get_instance());
        if (!unit) {
            FAIL() << "Failed to get_instance";
            return;
        }
        ustr = m5::utility::formatString("%s:SPI", unit->deviceName());
        if (!begin()) {
            FAIL() << "Failed to begin " << ustr;
        }
    }

    virtual void TearDown() override
    {
    }

    virtual bool begin()
    {
        return Units.add(*unit, get_spi(), get_spi_settings()) && Units.begin();
    }

    //! @brief return m5::unit::Component-derived class instance
    virtual U* get_instance() = 0;
    //! @brief return SPISettings for the unit under test
    virtual SPISettings get_spi_settings() = 0;
    //! @brief return SPIClass to be used (default: SPI)
    virtual SPIClass& get_spi()
    {
        return SPI;
    }

    std::string ustr{};
    std::unique_ptr<U> unit{};
    m5::unit::UnitUnified Units;
};

}  // namespace googletest
}  // namespace unit
}  // namespace m5
#endif
