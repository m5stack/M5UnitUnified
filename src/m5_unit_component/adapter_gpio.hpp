/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_gpio.hpp
  @brief Adapters to treat M5HAL and GPIO in the same way
  @note  Currently handles GPIO directly, but will handle via M5HAL in the future
*/
#ifndef M5_UNIT_COMPONENT_ADAPTER_GPIO_HPP
#define M5_UNIT_COMPONENT_ADAPTER_GPIO_HPP

#include "identify_functions.hpp"
#include "types.hpp"
#include "adapter_base.hpp"

namespace m5 {
namespace unit {
namespace gpio {

/*!
  @brief Calculate clk_div from desired tick time (nanoseconds)
  @param apb_freq_hz Current APB clock (Hz)
  @param tick_ns Desired tick time (ns)
  @return clk_div for RMT v1
*/
uint8_t calculate_rmt_clk_div(const uint32_t apb_freq_hz, const uint32_t tick_ns);

/*!
   @brief Calculate resolution from desired tick time
  @param apb_freq_hz Current APB clock (Hz)
  @param tick_ns Desired tick time (ns)
  @return resoution for RMT v2
*/
uint32_t calculate_rmt_resolution_hz(const uint32_t apb_freq_hz, const uint32_t tick_ns);

}  // namespace gpio

// Base class for AdapterGPIO
class AdapterGPIOBase : public Adapter {
public:
    class GPIOImpl : public Adapter::Impl {
    public:
        GPIOImpl() = default;
        GPIOImpl(const int8_t rx_pin, const int8_t tx_pin) : _rx_pin{(gpio_num_t)rx_pin}, _tx_pin{(gpio_num_t)tx_pin}
        {
        }
        inline gpio_num_t rx_pin() const
        {
            return _rx_pin;
        }
        inline gpio_num_t tx_pin() const
        {
            return _tx_pin;
        }
        inline gpio::adapter_config_t adapter_config() const
        {
            return _adapter_cfg;
        }

        inline virtual bool begin(const gpio::adapter_config_t& cfg)
        {
            return false;
        }

        //
        inline virtual m5::hal::error::error_t pinModeRX(const gpio::Mode m) override
        {
            return pin_mode(rx_pin(), m);
        }
        inline virtual m5::hal::error::error_t writeDigitalRX(const bool high) override
        {
            return write_digital(rx_pin(), high);
        }
        inline virtual m5::hal::error::error_t readDigitalRX(bool& high) override
        {
            return read_digital(rx_pin(), high);
        }
        inline virtual m5::hal::error::error_t writeAnalogRX(const uint16_t v) override
        {
            return write_analog(rx_pin(), v);
        }
        inline virtual m5::hal::error::error_t readAnalogRX(uint16_t& v)
        {
            return read_analog(v, rx_pin());
        }

        inline virtual m5::hal::error::error_t pulseInRX(uint32_t& duration, const int state,
                                                         const uint32_t timeout_us = 30000) override
        {
            return pulse_in(duration, rx_pin(), state, timeout_us);
        }

        //
        inline virtual m5::hal::error::error_t pinModeTX(const gpio::Mode m) override
        {
            return pin_mode(tx_pin(), m);
        }
        inline virtual m5::hal::error::error_t writeDigitalTX(const bool high) override
        {
            return write_digital(tx_pin(), high);
        }
        inline virtual m5::hal::error::error_t readDigitalTX(bool& high) override
        {
            return read_digital(tx_pin(), high);
        }
        inline virtual m5::hal::error::error_t writeAnalogTX(const uint16_t v) override
        {
            return write_analog(tx_pin(), v);
        }
        inline virtual m5::hal::error::error_t readAnalogTX(uint16_t& v)
        {
            return read_analog(v, tx_pin());
        }
        inline virtual m5::hal::error::error_t pulseInTX(uint32_t& duration, const int state,
                                                         const uint32_t timeout_us = 30000) override
        {
            return pulse_in(duration, tx_pin(), state, timeout_us);
        }

    protected:
        m5::hal::error::error_t pin_mode(const gpio_num_t pin, const gpio::Mode m);
        m5::hal::error::error_t write_digital(const gpio_num_t pin, const bool high);
        m5::hal::error::error_t read_digital(const gpio_num_t pin, bool& high);
        m5::hal::error::error_t write_analog(const gpio_num_t pin, const uint16_t value);
        m5::hal::error::error_t read_analog(uint16_t& value, const gpio_num_t pin);
        m5::hal::error::error_t pulse_in(uint32_t& duration, const gpio_num_t pin, const int state,
                                         const uint32_t timeout_us);

    protected:
        gpio_num_t _rx_pin{(gpio_num_t)-1}, _tx_pin{(gpio_num_t)-1};
        gpio::adapter_config_t _adapter_cfg{};
    };
    //
    explicit AdapterGPIOBase(GPIOImpl* impl);

    inline GPIOImpl* impl()
    {
        return static_cast<GPIOImpl*>(_impl.get());
    }
    inline const GPIOImpl* impl() const
    {
        return static_cast<GPIOImpl*>(_impl.get());
    }
    inline gpio_num_t rx_pin() const
    {
        return impl()->rx_pin();
    }
    inline gpio_num_t tx_pin() const
    {
        return impl()->tx_pin();
    }
    inline bool begin(const gpio::adapter_config_t& cfg)
    {
        return impl()->begin(cfg);
    }
};

}  // namespace unit
}  // namespace m5

#endif
