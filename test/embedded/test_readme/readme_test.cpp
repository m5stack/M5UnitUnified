/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Verify that README.md sample code patterns compile and work correctly.
  Uses UnitDummy* classes as substitutes for real units.
*/
#include <gtest/gtest.h>
#include <M5UnitComponent.hpp>
#include <M5UnitUnified.hpp>
#include <m5_unit_component/adapter.hpp>
#include "unit_dummy.hpp"
#include <Wire.h>
#include <SPI.h>
#include <M5Unified.h>
#include <M5HAL.hpp>
#include <esp32-hal-i2c.h>

using namespace m5::unit;

// README pattern: Unit using Wire
TEST(ReadmeSample, Wire)
{
    UnitUnified Units;
    UnitDummy unit;

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    if (i2cIsInit(0)) {
        Wire.end();
    }
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    EXPECT_TRUE(Units.add(unit, Wire));
    EXPECT_EQ(unit.adapter()->type(), Adapter::Type::I2C);
    EXPECT_TRUE(Units.begin());

    Units.update();
    EXPECT_GT(unit.count, 0U);
}

// README pattern: Unit using I2C_Class (M5Unified In_I2C)
TEST(ReadmeSample, I2C_Class)
{
    UnitUnified Units;
    UnitDummy unit;

    EXPECT_TRUE(Units.add(unit, M5.In_I2C));
    EXPECT_EQ(unit.adapter()->type(), Adapter::Type::I2C);
    EXPECT_TRUE(Units.begin());

    Units.update();
    EXPECT_GT(unit.count, 0U);
}

// README pattern: Unit using M5HAL Bus (SoftwareI2C)
TEST(ReadmeSample, M5HAL_Bus)
{
    UnitUnified Units;
    UnitDummy unit;

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);

    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

    EXPECT_TRUE(Units.add(unit, i2c_bus ? i2c_bus.value() : nullptr));
    EXPECT_TRUE(Units.begin());

    Units.update();
    EXPECT_GT(unit.count, 0U);
}

// README pattern: Unit using GPIO
TEST(ReadmeSample, GPIO)
{
    UnitUnified Units;
    UnitDummyGPIO unit;

    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGW("PortB is not available, using PortA");
        Wire.end();
        pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

    EXPECT_TRUE(Units.add(unit, pin_num_gpio_in, pin_num_gpio_out));
    EXPECT_EQ(unit.adapter()->type(), Adapter::Type::GPIO);
    EXPECT_TRUE(Units.begin());
}

// README pattern: Unit using UART(Serial)
TEST(ReadmeSample, UART)
{
    UnitUnified Units;
    UnitDummyUART unit;

    auto pin_num_in  = M5.getPin(m5::pin_name_t::port_c_rxd);
    auto pin_num_out = M5.getPin(m5::pin_name_t::port_c_txd);
    if (pin_num_in < 0 || pin_num_out < 0) {
        M5_LOGW("PortC is not available, using PortA");
        Wire.end();
        pin_num_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }

#if SOC_UART_NUM > 2
    auto& s = Serial2;
#elif SOC_UART_NUM > 1
    auto& s = Serial1;
#else
#error "Not enough Serial"
#endif
    s.end();
    s.begin(19200, SERIAL_8N1, pin_num_in, pin_num_out);

    EXPECT_TRUE(Units.add(unit, s));
    EXPECT_EQ(unit.adapter()->type(), Adapter::Type::UART);
    EXPECT_TRUE(Units.begin());
}

// README pattern: Unit using SPI
TEST(ReadmeSample, SPI)
{
    UnitUnified Units;
    UnitDummySPI unit;

    SPISettings settings = {10000000, MSBFIRST, SPI_MODE1};
    EXPECT_TRUE(Units.add(unit, ::SPI, settings));
    EXPECT_EQ(unit.adapter()->type(), Adapter::Type::SPI);
    EXPECT_TRUE(Units.begin());
}
