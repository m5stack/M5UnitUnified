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
#ifndef M5_UNIT_COMPONENT_PIN_HPP
#define M5_UNIT_COMPONENT_PIN_HPP
#include <cstdint>

namespace m5 {
namespace unit {
namespace gpio {

// From M5GFX
class pin_backup_t {
public:
    explicit pin_backup_t(int pin_num = 1);
    inline void setPin(int pin_num)
    {
        _pin_num = pin_num;
    }
    inline int8_t getPin(void) const
    {
        return _pin_num;
    }
    void backup(void);
    void restore(void);

private:
    uint32_t _io_mux_gpio_reg;
    uint32_t _gpio_pin_reg;
    uint32_t _gpio_func_out_reg;
    uint32_t _gpio_func_in_reg;
    int16_t _in_func_num = -1;
    int8_t _pin_num      = -1;  // GPIO_NUM_NC
    bool _gpio_enable;
};

}  // namespace gpio
}  // namespace unit
}  // namespace m5
#endif
