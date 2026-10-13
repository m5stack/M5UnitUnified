/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file pin.cpp
  @brief PIN settings save/restore
  @todo Will be transferred to M5HAL in the future
*/
#include "pin.hpp"
#include <M5Utility.hpp>
#include <cstddef>
#if defined(ESP_PLATFORM)
#include <driver/i2c.h>
#include <soc/gpio_struct.h>
#include <soc/gpio_periph.h>
#include <soc/gpio_reg.h>
#endif

#if defined(ESP_PLATFORM)
#include <soc/io_mux_reg.h>
// GPIO_PIN_MUX_REG[] is declared by ESP-IDF but has no definition on chips with a contiguous IO_MUX
// (ESP32-C5 / C61 etc.); fall back to computing the address from IO_MUX_GPIO0_REG
extern "C" const uint32_t GPIO_PIN_MUX_REG[] __attribute__((weak));
namespace {
uint32_t io_mux_reg(const size_t pin)
{
    return GPIO_PIN_MUX_REG ? GPIO_PIN_MUX_REG[pin] : (IO_MUX_GPIO0_REG + static_cast<uint32_t>(pin) * 4u);
}
// GPIO number field of the input-signal select register (ESP32-P4 reserves input signal 0, so its macros start at 1)
// and the bit that routes the signal through the GPIO matrix
#if defined(GPIO_FUNC0_IN_SEL_S)
constexpr uint32_t in_sel_shift  = GPIO_FUNC0_IN_SEL_S;
constexpr uint32_t in_sel_mask   = GPIO_FUNC0_IN_SEL_V;
constexpr uint32_t in_sel_matrix = GPIO_SIG0_IN_SEL;
#else
constexpr uint32_t in_sel_shift  = GPIO_FUNC1_IN_SEL_S;
constexpr uint32_t in_sel_mask   = GPIO_FUNC1_IN_SEL_V;
constexpr uint32_t in_sel_matrix = GPIO_SIG1_IN_SEL;
#endif
}  // namespace
#endif

namespace m5 {
namespace unit {
namespace gpio {

#if defined(ESP_PLATFORM)
pin_backup_t::pin_backup_t(int pin_num) : _pin_num{static_cast<gpio_num_t>(pin_num)}
{
    if (pin_num >= 0) {
        backup();
    }
}

void pin_backup_t::backup(void)
{
    auto pin_num = (size_t)_pin_num;
    if (pin_num < GPIO_NUM_MAX) {
        _io_mux_gpio_reg   = *reinterpret_cast<uint32_t*>(io_mux_reg(pin_num));
        _gpio_pin_reg      = *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4));
        _gpio_func_out_reg = *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4));

#if defined(GPIO_ENABLE1_REG)
        _gpio_enable = (bool)((*reinterpret_cast<uint32_t*>(((pin_num & 32) ? GPIO_ENABLE1_REG : GPIO_ENABLE_REG)) &
                               (1U << (pin_num & 31))) != 0);
#else
        _gpio_enable = (bool)((*reinterpret_cast<uint32_t*>(GPIO_ENABLE_REG) & (1U << (pin_num & 31))) != 0);
#endif

        _in_func_num = -1;

        size_t func_num = ((_gpio_func_out_reg >> GPIO_FUNC0_OUT_SEL_S) & GPIO_FUNC0_OUT_SEL_V);
        if (func_num < sizeof(GPIO.func_in_sel_cfg) / sizeof(GPIO.func_in_sel_cfg[0])) {
            _gpio_func_in_reg = GPIO.func_in_sel_cfg[func_num].val;
            // The input of the same signal is routed from this pin (e.g. I2C SDA/SCL, whose in/out indexes match).
            // The selector counts only while the signal goes through the GPIO matrix; otherwise it is a stale value
            if ((_gpio_func_in_reg & in_sel_matrix) && pin_num == ((_gpio_func_in_reg >> in_sel_shift) & in_sel_mask)) {
                _in_func_num = func_num;
                M5_LIB_LOGV("backup pin:%d : func_num:%d", pin_num, _in_func_num);
            }
        }
    }
}

void pin_backup_t::restore(void)
{
    auto pin_num = (size_t)_pin_num;
    if (pin_num < GPIO_NUM_MAX) {
        M5_LIB_LOGV("restore pin:%d ", pin_num);
        M5_LIB_LOGV("restore IO_MUX_GPIO0_REG          :%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(io_mux_reg(pin_num)), _io_mux_gpio_reg);
        M5_LIB_LOGV("restore GPIO_PIN0_REG             :%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4)), _gpio_pin_reg);
        M5_LIB_LOGV("restore GPIO_FUNC0_OUT_SEL_CFG_REG:%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4)), _gpio_func_out_reg);

        *reinterpret_cast<uint32_t*>(io_mux_reg(_pin_num))                       = _io_mux_gpio_reg;
        *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4))              = _gpio_pin_reg;
        *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4)) = _gpio_func_out_reg;

        // Route the input back only after the pad configuration, so it is not reconnected to a pad
        // still set up by the borrower
        if ((uint16_t)_in_func_num < 256) {
            GPIO.func_in_sel_cfg[_in_func_num].val = _gpio_func_in_reg;
            M5_LIB_LOGV("pin:%d in_func_num:%d", (int)pin_num, (int)_in_func_num);
        }

#if defined(GPIO_ENABLE1_REG)
        auto gpio_enable_reg = reinterpret_cast<uint32_t*>(((pin_num & 32) ? GPIO_ENABLE1_REG : GPIO_ENABLE_REG));
#else
        auto gpio_enable_reg = reinterpret_cast<uint32_t*>(GPIO_ENABLE_REG);
#endif

        uint32_t pin_mask = 1 << (pin_num & 31);
        uint32_t val      = *gpio_enable_reg;
        M5_LIB_LOGV("restore GPIO_ENABLE_REG:%08x", (int)*gpio_enable_reg);
        if (_gpio_enable) {
            val |= pin_mask;
        } else {
            val &= ~pin_mask;
        }
        *gpio_enable_reg = val;
    }
}
#else
// GPIO register access is unavailable on non-ESP builds; keep the API and do nothing
pin_backup_t::pin_backup_t(int pin_num) : _pin_num{static_cast<int8_t>(pin_num)}
{
}

void pin_backup_t::backup(void)
{
}

void pin_backup_t::restore(void)
{
}
#endif

}  // namespace gpio
}  // namespace unit
}  // namespace m5
