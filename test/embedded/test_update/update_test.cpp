/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitComponent
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
#include <wiring/m5_unit_unified_wiring.hpp>  // include last; uses M5Unified/M5HAL detected above

using namespace m5::unit;

constexpr uint32_t I2C_FREQ{400000U};

// Board-aware connection helpers delegate to the m5::unit::wiring helper. Dogfooding it here also
// compile-checks the header and exercises it on real hardware.
static bool add_with_i2c(UnitUnified& units, Component& u)
{
    return m5::unit::wiring::addI2C(units, u, I2C_FREQ);  // default PortB = SoftwareI2C on NessoN1
}

static bool add_with_gpio(UnitUnified& units, Component& u)
{
    return m5::unit::wiring::addGPIO(units, u);
}

static bool add_with_uart(UnitUnified& units, Component& u)
{
    return m5::unit::wiring::addUART(units, u);
}

static bool add_with_spi(UnitUnified& units, Component& u)
{
    SPISettings settings{1000000, MSBFIRST, SPI_MODE0};
    return m5::unit::wiring::spiBus(units, u, SPI, settings);
}

// Test: I2C unit add/begin/update lifecycle (success path)
TEST(Component, Update)
{
    UnitUnified units;
    UnitDummy u;
    EXPECT_FALSE(u.isRegistered());

    {
        auto cfg = u.component_config();
        EXPECT_FALSE(cfg.self_update);  // false as default

        EXPECT_EQ(u.count, 0U);
        EXPECT_TRUE(add_with_i2c(units, u));

        units.update();  // Dont call u.update() because unit was not begun.
        EXPECT_EQ(u.count, 0U);

        EXPECT_TRUE(units.begin());
        units.update();  // Call u.update()
        EXPECT_EQ(u.count, 1U);

        cfg.self_update = true;
        u.component_config(cfg);
        cfg = u.component_config();
        EXPECT_TRUE(cfg.self_update);

        units.update();  // Don't call u.update()
        EXPECT_EQ(u.count, 1U);

        u.update();  // If component_config.self_update is true, you have to call it yourself
        EXPECT_EQ(u.count, 2U);
    }
}

// Test: assign should fail for units with wrong access attribute
TEST(Component, AssignFailMismatch)
{
    UnitUnified units;

    // GPIO unit should fail to assign via I2C
    {
        UnitDummyGPIO gpio_unit;
        EXPECT_TRUE(gpio_unit.canAccessGPIO());
        EXPECT_FALSE(gpio_unit.canAccessI2C());
        EXPECT_FALSE(add_with_i2c(units, gpio_unit));
    }

    // UART unit should fail to assign via I2C
    {
        UnitDummyUART uart_unit;
        EXPECT_TRUE(uart_unit.canAccessUART());
        EXPECT_FALSE(uart_unit.canAccessI2C());
        EXPECT_FALSE(add_with_i2c(units, uart_unit));
    }

    // SPI unit should fail to assign via I2C
    {
        UnitDummySPI spi_unit;
        EXPECT_TRUE(spi_unit.canAccessSPI());
        EXPECT_FALSE(spi_unit.canAccessI2C());
        EXPECT_FALSE(add_with_i2c(units, spi_unit));
    }

    // Unit with no access attribute should fail
    {
        UnitDummyNone none_unit;
        EXPECT_FALSE(none_unit.canAccessI2C());
        EXPECT_FALSE(none_unit.canAccessGPIO());
        EXPECT_FALSE(none_unit.canAccessUART());
        EXPECT_FALSE(none_unit.canAccessSPI());
        EXPECT_FALSE(add_with_i2c(units, none_unit));
    }
}

// Test: units with combined access attributes should succeed on matching bus
TEST(Component, AssignSuccessCombined)
{
    UnitUnified units;

    // I2C+SPI unit should succeed on I2C
    {
        UnitDummyI2CSPI i2cspi_unit;
        EXPECT_TRUE(i2cspi_unit.canAccessI2C());
        EXPECT_TRUE(i2cspi_unit.canAccessSPI());
        EXPECT_FALSE(i2cspi_unit.canAccessGPIO());
        EXPECT_FALSE(i2cspi_unit.canAccessUART());
        EXPECT_TRUE(add_with_i2c(units, i2cspi_unit));
    }

    // All-access unit should succeed on I2C
    {
        UnitDummyAll all_unit;
        EXPECT_TRUE(all_unit.canAccessI2C());
        EXPECT_TRUE(all_unit.canAccessGPIO());
        EXPECT_TRUE(all_unit.canAccessUART());
        EXPECT_TRUE(all_unit.canAccessSPI());
        EXPECT_TRUE(add_with_i2c(units, all_unit));
    }
}

// Test: GPIO unit add success
TEST(Component, AddGPIO)
{
    UnitUnified units;
    UnitDummyGPIO u;
    EXPECT_TRUE(u.canAccessGPIO());
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::Unknown);
    EXPECT_TRUE(add_with_gpio(units, u));
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::GPIO);
}

// Test: UART unit add success
TEST(Component, AddUART)
{
    UnitUnified units;
    UnitDummyUART u;
    EXPECT_TRUE(u.canAccessUART());
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::Unknown);
    EXPECT_TRUE(add_with_uart(units, u));
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::UART);
}

// Test: SPI unit add success
TEST(Component, AddSPI)
{
    UnitUnified units;
    UnitDummySPI u;
    EXPECT_TRUE(u.canAccessSPI());
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::Unknown);
    EXPECT_TRUE(add_with_spi(units, u));
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::SPI);
}

// Test: cross-bus mismatch failures (GPIO/UART/SPI units on wrong bus)
TEST(Component, CrossBusMismatch)
{
    // I2C unit should fail on GPIO/UART/SPI
    {
        UnitUnified units;
        UnitDummy i2c_unit;
        EXPECT_FALSE(add_with_gpio(units, i2c_unit));
    }
    {
        UnitUnified units;
        UnitDummy i2c_unit;
        EXPECT_FALSE(add_with_uart(units, i2c_unit));
    }
    {
        UnitUnified units;
        UnitDummy i2c_unit;
        EXPECT_FALSE(add_with_spi(units, i2c_unit));
    }

    // GPIO unit should fail on I2C/UART/SPI
    {
        UnitUnified units;
        UnitDummyGPIO gpio_unit;
        EXPECT_FALSE(add_with_i2c(units, gpio_unit));
    }
    {
        UnitUnified units;
        UnitDummyGPIO gpio_unit;
        EXPECT_FALSE(add_with_uart(units, gpio_unit));
    }
    {
        UnitUnified units;
        UnitDummyGPIO gpio_unit;
        EXPECT_FALSE(add_with_spi(units, gpio_unit));
    }

    // UART unit should fail on I2C/GPIO/SPI
    {
        UnitUnified units;
        UnitDummyUART uart_unit;
        EXPECT_FALSE(add_with_i2c(units, uart_unit));
    }
    {
        UnitUnified units;
        UnitDummyUART uart_unit;
        EXPECT_FALSE(add_with_gpio(units, uart_unit));
    }
    {
        UnitUnified units;
        UnitDummyUART uart_unit;
        EXPECT_FALSE(add_with_spi(units, uart_unit));
    }

    // SPI unit should fail on I2C/GPIO/UART
    {
        UnitUnified units;
        UnitDummySPI spi_unit;
        EXPECT_FALSE(add_with_i2c(units, spi_unit));
    }
    {
        UnitUnified units;
        UnitDummySPI spi_unit;
        EXPECT_FALSE(add_with_gpio(units, spi_unit));
    }
    {
        UnitUnified units;
        UnitDummySPI spi_unit;
        EXPECT_FALSE(add_with_uart(units, spi_unit));
    }
}

// Test: combined-access units on matching buses
TEST(Component, CombinedAccessMultiBus)
{
    // I2C+SPI unit should succeed on both I2C and SPI
    {
        UnitUnified units;
        UnitDummyI2CSPI u;
        EXPECT_TRUE(add_with_i2c(units, u));
    }
    {
        UnitUnified units;
        UnitDummyI2CSPI u;
        EXPECT_TRUE(add_with_spi(units, u));
    }
    // I2C+SPI unit should fail on GPIO and UART
    {
        UnitUnified units;
        UnitDummyI2CSPI u;
        EXPECT_FALSE(add_with_gpio(units, u));
    }
    {
        UnitUnified units;
        UnitDummyI2CSPI u;
        EXPECT_FALSE(add_with_uart(units, u));
    }

    // All-access unit should succeed on everything
    {
        UnitUnified units;
        UnitDummyAll u;
        EXPECT_TRUE(add_with_i2c(units, u));
    }
    {
        UnitUnified units;
        UnitDummyAll u;
        EXPECT_TRUE(add_with_gpio(units, u));
    }
    {
        UnitUnified units;
        UnitDummyAll u;
        EXPECT_TRUE(add_with_uart(units, u));
    }
    {
        UnitUnified units;
        UnitDummyAll u;
        EXPECT_TRUE(add_with_spi(units, u));
    }
}

// Test: order and adapter after add/begin
TEST(Component, OrderAndAdapter)
{
    UnitUnified units;
    UnitDummy u;

    EXPECT_EQ(u.order(), 0U);
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::Unknown);

    EXPECT_TRUE(add_with_i2c(units, u));

    // After add, order should be > 0 and adapter type should be I2C
    EXPECT_GT(u.order(), 0U);
    EXPECT_EQ(u.adapter()->type(), Adapter::Type::I2C);
}

// Test: duplicate add should fail
TEST(Component, DuplicateAdd)
{
    UnitUnified units;
    UnitDummy u;

    EXPECT_TRUE(add_with_i2c(units, u));
    EXPECT_TRUE(u.isRegistered());
    // Second add of same unit should fail
    EXPECT_FALSE(add_with_i2c(units, u));
}
