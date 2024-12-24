/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file pin.hpp
  @brief PIN settings save/restore
  @todo Will be transferred to M5HAL in the future
*/
#include "pin.hpp"
#include <M5Utility.hpp>
#include <cstddef>
#include <driver/i2c.h>
#include <soc/gpio_struct.h>
#include <soc/gpio_periph.h>

namespace m5 {
namespace unit {
namespace gpio {

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
        _io_mux_gpio_reg   = *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[pin_num]);
        _gpio_pin_reg      = *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4));
        _gpio_func_out_reg = *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4));

#if defined(GPIO_ENABLE1_REG)
        _gpio_enable = (bool)((*reinterpret_cast<uint32_t*>(((pin_num & 32) ? GPIO_ENABLE1_REG : GPIO_ENABLE_REG)) &
                               (1U << (pin_num & 31))) != 0);
#else
        _gpio_enable = (bool)((*reinterpret_cast<uint32_t*>(GPIO_ENABLE_REG) & (1U << (pin_num & 31)) != 0);
#endif

        _in_func_num = -1;

        size_t func_num = ((_gpio_func_out_reg >> GPIO_FUNC0_OUT_SEL_S) & GPIO_FUNC0_OUT_SEL_V);
        if (func_num < sizeof(GPIO.func_in_sel_cfg) / sizeof(GPIO.func_in_sel_cfg[0])) {
            _gpio_func_in_reg = *reinterpret_cast<uint32_t*>(GPIO_FUNC0_IN_SEL_CFG_REG + (func_num * 4));
            if (func_num == ((_gpio_func_in_reg >> GPIO_FUNC0_IN_SEL_S) & GPIO_FUNC0_IN_SEL_V)) {
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
        if ((uint16_t)_in_func_num < 256) {
            GPIO.func_in_sel_cfg[_in_func_num].val = _gpio_func_in_reg;
            M5_LIB_LOGV("pin:%d in_func_num:%d", (int)pin_num, (int)_in_func_num);
        }

        M5_LIB_LOGV("restore pin:%d ", pin_num);
        M5_LIB_LOGV("restore IO_MUX_GPIO0_REG          :%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[pin_num]), _io_mux_gpio_reg);
        M5_LIB_LOGV("restore GPIO_PIN0_REG             :%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4)), _gpio_pin_reg);
        M5_LIB_LOGV("restore GPIO_FUNC0_OUT_SEL_CFG_REG:%08x -> %08x ",
                    *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4)), _gpio_func_out_reg);

        *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[_pin_num])                 = _io_mux_gpio_reg;
        *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG + (pin_num * 4))              = _gpio_pin_reg;
        *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4)) = _gpio_func_out_reg;

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

}  // namespace gpio
}  // namespace unit
}  // namespace m5
