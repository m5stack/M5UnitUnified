/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_gpio.cpp
  @brief Adapters to treat M5HAL and RMT in the same way
  @note  Currently handles GPIO directly, but will handle via M5HAL in the future
*/
#include "adapter_gpio.hpp"
#include <driver/gpio.h>
#include <esp_idf_version.h>

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
#pragma message "Using RMT v2,Oneshot"
#include <esp_adc/adc_oneshot.h>
#else
#pragma message "Using RMT v1"
#include <driver/adc.h>
#endif

// ADC_ATTEN_DB_12 was introduced in ESP-IDF v4.4.7 / v5.1.3
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3) || \
    (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 7) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
#pragma message "Exists ADC_ATTEN_DB_12"
constexpr auto M5_ADC_ATTEN_DB = ADC_ATTEN_DB_12;
#else
#pragma message "Not exists ADC_ATTEN_DB_12"
constexpr auto M5_ADC_ATTEN_DB = ADC_ATTEN_DB_11;
#endif

#if defined(SOC_DAC_SUPPORTED) && SOC_DAC_SUPPORTED
#pragma message "DAC supported"

#if __has_include(<driver/dac_common.h>)
#include <driver/dac_common.h>
#define USING_RMT_CHANNNE_T
#endif

#endif

#if !defined(USING_RMT_CHANNNE_T) && defined(ARDUINO)
#include <esp32-hal-dac.h>
#endif

#if SOC_ADC_SUPPORTED
#pragma message "ADC supported"
#else
#pragma message "ADC Not supported"
#endif

#include <esp_timer.h>

namespace {
constexpr gpio_config_t gpio_cfg_table[] = {
    {
        // Input
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // Output
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // Pullup
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // InputPullup
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // Pulldown
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // InputPulldown
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // OpenDrain
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_ENABLE,  //
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // OutputOpenDrain,
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
    {
        // Analog
        .pin_bit_mask = 0,
        .mode         = GPIO_MODE_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
#if defined(CONFIG_IDF_TARGET_ESP32P4)
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE,
#endif
    },
};

#if CONFIG_IDF_TARGET_ESP32
constexpr int8_t gpio_to_adc_table[] = {
    /*  0 */ 11,  // ADC2_CHANNEL_1
    /*  1 */ -1,
    /*  2 */ 12,  // ADC2_CHANNEL_2
    /*  3 */ -1,
    /*  4 */ 10,  // ADC2_CHANNEL_0
    /*  5 */ -1,
    /*  6 */ -1,
    /*  7 */ -1,
    /*  8 */ -1,
    /*  9 */ -1,
    /* 10 */ -1,
    /* 11 */ -1,
    /* 12 */ 15,  // ADC2_CHANNEL_5
    /* 13 */ 14,  // ADC2_CHANNEL_4
    /* 14 */ 16,  // ADC2_CHANNEL_6
    /* 15 */ 13,  // ADC2_CHANNEL_3
    /* 16 */ -1,
    /* 17 */ -1,
    /* 18 */ -1,
    /* 19 */ -1,
    /* 20 */ -1,
    /* 21 */ -1,
    /* 22 */ -1,
    /* 23 */ -1,
    /* 24 */ -1,
    /* 25 */ 18,  // ADC2_CHANNEL_8
    /* 26 */ 19,  // ADC2_CHANNEL_9
    /* 27 */ 17,  // ADC2_CHANNEL_7
    /* 28 */ -1,
    /* 29 */ -1,
    /* 30 */ -1,
    /* 31 */ -1,
    /* 32 */ 4,  // ADC1_CHANNEL_4
    /* 33 */ 5,  // ADC1_CHANNEL_5
    /* 34 */ 6,  // ADC1_CHANNEL_6
    /* 35 */ 7,  // ADC1_CHANNEL_7
    /* 36 */ 0,  // ADC1_CHANNEL_0
    /* 37 */ 1,  // ADC1_CHANNEL_1
    /* 38 */ 2,  // ADC1_CHANNEL_2
    /* 39 */ 3   // ADC1_CHANNEL_3
};
#elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
constexpr int8_t gpio_to_adc_table[] = {
    /*  0 */ -1,
    /*  1 */ 0,   // ADC1_CHANNEL_0
    /*  2 */ 1,   // ADC1_CHANNEL_1
    /*  3 */ 2,   // ADC1_CHANNEL_2
    /*  4 */ 3,   // ADC1_CHANNEL_3
    /*  5 */ 4,   // ADC1_CHANNEL_4
    /*  6 */ 5,   // ADC1_CHANNEL_5
    /*  7 */ 6,   // ADC1_CHANNEL_6
    /*  8 */ 7,   // ADC1_CHANNEL_7
    /*  9 */ 8,   // ADC1_CHANNEL_8
    /* 10 */ 9,   // ADC1_CHANNEL_9
    /* 11 */ 10,  // ADC2_CHANNEL_0
    /* 12 */ 11,  // ADC2_CHANNEL_1
    /* 13 */ 12,  // ADC2_CHANNEL_2
    /* 14 */ 13,  // ADC2_CHANNEL_3
    /* 15 */ 14,  // ADC2_CHANNEL_4
    /* 16 */ 15,  // ADC2_CHANNEL_5
    /* 17 */ 16,  // ADC2_CHANNEL_6
    /* 18 */ 17,  // ADC2_CHANNEL_7
    /* 19 */ 18,  // ADC2_CHANNEL_8
    /* 20 */ 19,  // ADC2_CHANNEL_9
};
#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2
constexpr int8_t gpio_to_adc_table[] = {
    /*  0 */ 0,   // ADC1_CHANNEL_0
    /*  1 */ 1,   // ADC1_CHANNEL_1
    /*  2 */ 2,   // ADC1_CHANNEL_2
    /*  3 */ 3,   // ADC1_CHANNEL_3
    /*  4 */ 4,   // ADC1_CHANNEL_4
    /*  5 */ 10,  // ADC2_CHANNEL_0
};
#elif CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32P4 || \
    CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C61
constexpr int8_t gpio_to_adc_table[] = {
    /*  0 */ 0,  // ADC1_CHANNEL_0
    /*  1 */ 1,  // ADC1_CHANNEL_1
    /*  2 */ 2,  // ADC1_CHANNEL_2
    /*  3 */ 3,  // ADC1_CHANNEL_3
    /*  4 */ 4,  // ADC1_CHANNEL_4
    /*  5 */ 5,  // ADC1_CHANNEL_5
    /*  6 */ 6,  // ADC1_CHANNEL_6
};
#elif CONFIG_IDF_TARGET_ESP32P4
constexpr int8_t gpio_to_adc_table[] = {
    /*  0 */ -1,
    /*  1 */ -1,
    /*  2 */ -1,
    /*  3 */ -1,
    /*  4 */ -1,
    /*  5 */ -1,
    /*  6 */ -1,
    /*  7 */ -1,
    /*  8 */ -1,
    /*  9 */ -1,
    /* 10 */ -1,
    /* 11 */ -1,
    /* 12 */ -1,
    /* 13 */ -1,
    /* 14 */ -1,
    /* 15 */ -1,
    /* 16 */ 0,  // ADC1_CHANNEL_0
    /* 17 */ 1,  // ADC1_CHANNEL_1
    /* 18 */ 2,  // ADC1_CHANNEL_2
    /* 19 */ 3,  // ADC1_CHANNEL_3
    /* 20 */ 4,  // ADC1_CHANNEL_4
    /* 21 */ 5,  // ADC1_CHANNEL_5
    /* 22 */ 6,  // ADC1_CHANNEL_6
    /* 23 */ 7,  // ADC1_CHANNEL_7
    /* 24 */ -1,
    /* 25 */ -1,
    /* 26 */ -1,
    /* 27 */ -1,
    /* 28 */ -1,
    /* 29 */ -1,
    /* 30 */ -1,
    /* 31 */ -1,
    /* 32 */ -1,
    /* 33 */ -1,
    /* 34 */ -1,
    /* 35 */ -1,
    /* 36 */ -1,
    /* 37 */ -1,
    /* 38 */ -1,
    /* 39 */ -1,
    /* 40 */ -1,
    /* 41 */ -1,
    /* 42 */ -1,
    /* 43 */ -1,
    /* 44 */ -1,
    /* 45 */ -1,
    /* 46 */ -1,
    /* 47 */ -1,
    /* 48 */ -1,
    /* 49 */ 10,  // ADC2_CHANNEL_0
    /* 50 */ 11,  // ADC2_CHANNEL_1
    /* 51 */ 12,  // ADC2_CHANNEL_2
    /* 52 */ 13,  // ADC2_CHANNEL_3
    /* 53 */ 14,  // ADC2_CHANNEL_4
    /* 54 */ 15,  // ADC2_CHANNEL_5
};
#else
#error Invalid target
#endif

// 0-9: ADC1 10-:ADC2 (ESP-IDF 4.x)
// 0-9, 10- : ADC (ESP-IDF 5.x)
int8_t gpio_to_adc_channel(const int8_t pin)
{
    if (pin < 0 || pin >= m5::stl::size(gpio_to_adc_table)) {
        return -1;
    }
    auto v = gpio_to_adc_table[pin];
    return (v < 10) ? v : v - 10;
}

#if 0
// -1:ivalid 0:ADC1 1:ADC2
int gpio_to_adc12(const int8_t pin)
{
    return (pin >= 0 && pin < m5::stl::size(gpio_to_adc_table))
               ? (gpio_to_adc_table[pin] < 0 ? -1 : (gpio_to_adc_table[pin] >= 10 ? 1 : 0))
               : -1;
}
#endif

}  // namespace

namespace m5 {
namespace unit {

namespace gpio {

uint8_t calculate_rmt_clk_div(uint32_t apb_freq_hz, uint32_t tick_ns)
{
    if (tick_ns == 0) {
        return 1;
    }

    uint64_t clk_div = (static_cast<uint64_t>(apb_freq_hz) * tick_ns + 500) / 1000000000UL;
    clk_div          = std::min<uint64_t>(255, std::max<uint64_t>(0, clk_div));
    return static_cast<uint8_t>(clk_div);
}

uint32_t calculate_rmt_resolution_hz(uint32_t apb_freq_hz, uint32_t tick_ns)
{
    if (tick_ns == 0) {
        return apb_freq_hz;
    }

    uint64_t target_hz = 1000000000UL / tick_ns;
    uint32_t clk_div   = std::max<uint32_t>(1, (apb_freq_hz + target_hz / 2) / target_hz);
    clk_div            = std::min<uint32_t>(clk_div, 255U);
    return apb_freq_hz / clk_div;
}

}  // namespace gpio

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::pin_mode(const gpio_num_t pin, const gpio::Mode m)
{
    if (m < gpio::Mode::RmtRX) {
        gpio_config_t cfg = gpio_cfg_table[m5::stl::to_underlying(m)];
        cfg.pin_bit_mask  = 1ULL << pin;
        gpio_config(&cfg);
        return m5::hal::error::error_t::OK;
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::write_digital(const gpio_num_t pin, const bool high)
{
    gpio_set_level(pin, high);
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::read_digital(const gpio_num_t pin, bool& high)
{
    high = true;
    high = gpio_get_level(pin);
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::write_analog(const gpio_num_t pin, const uint16_t value)
{
    if (pin != 25 && pin != 26) {
        // DAC output can 25 or 26
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
#if defined(USING_RMT_CHANNNE_T)
    dac_channel_t ch = (pin == 25) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;
    dac_output_enable(ch);
    dac_output_voltage(ch, static_cast<uint8_t>(value & 0xFF));  // 0〜255
    return m5::hal::error::error_t::OK;
#else
    analogWrite(pin, value & 0xFF);
    return m5::hal::error::error_t::OK;
    //      return m5::hal::error::error_t::NOT_IMPLEMENTED;
#endif
}

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::read_analog(uint16_t& value, const gpio_num_t pin)
{
    value = 0;

    const auto ch = gpio_to_adc_channel(pin);
    if (ch < 0) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
#if !defined(SOC_ADC_PERIPH_NUM) || SOC_ADC_PERIPH_NUM <= 1
    if (ch >= 10) {
        M5_LIB_LOGE("Not support ADC2");
        return m5::hal::error::error_t::NOT_IMPLEMENTED;
    }
#endif

#if defined(M5_UNIT_UNIFIED_USING_ADC_ONESHOT)
    // ESP-IDF 5.x
    adc_unit_t unit       = (ch < 10) ? ADC_UNIT_1 : ADC_UNIT_2;
    adc_channel_t channel = static_cast<adc_channel_t>((ch < 10) ? ch : (ch - 10));

    adc_oneshot_unit_handle_t adc_handle{};
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit, .clk_src = ADC_DIGI_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};
#else
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit, .clk_src = ADC_RTC_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};
#endif

    if (adc_oneshot_new_unit(&init_config, &adc_handle) != ESP_OK) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten    = M5_ADC_ATTEN_DB,      // 0~3.3V
        .bitwidth = ADC_BITWIDTH_DEFAULT  // 12bit
    };

    auto ret = m5::hal::error::error_t::UNKNOWN_ERROR;
    if (adc_oneshot_config_channel(adc_handle, channel, &chan_config) == ESP_OK) {
        int raw{};
        if (adc_oneshot_read(adc_handle, channel, &raw) == ESP_OK) {
            value = static_cast<uint16_t>(raw);
            ret   = m5::hal::error::error_t::OK;
        }
    }
    adc_oneshot_del_unit(adc_handle);
    return ret;

#else
    // ESP-IDF 4.x
    // ADC2
    if (ch >= 10) {
#if SOC_ADC_SUPPORTED && SOC_ADC_PERIPH_NUM > 1
        adc2_channel_t channel = static_cast<adc2_channel_t>(ch - 10);
        int v                  = 0;
        if (adc2_get_raw(channel, ADC_WIDTH_BIT_12, &v) != ESP_OK) {
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        value = static_cast<uint16_t>(v);
        return m5::hal::error::error_t::OK;
#endif
    }
    // ADC1
    adc1_channel_t channel = static_cast<adc1_channel_t>(ch);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, M5_ADC_ATTEN_DB);
    value = static_cast<uint16_t>(adc1_get_raw(channel));
    return m5::hal::error::error_t::OK;
#endif
}

m5::hal::error::error_t AdapterGPIOBase::GPIOImpl::pulse_in(uint32_t& duration, const gpio_num_t pin, const int state,
                                                            const uint32_t timeout_us)
{
    duration   = 0;
    auto start = esp_timer_get_time();
    auto now   = start;

    // Wait for any previous pulse to end
    while (gpio_get_level(pin) == state) {
        now = esp_timer_get_time();
        if (now - start > timeout_us) {
            return m5::hal::error::error_t::TIMEOUT_ERROR;
        }
    }

    // Wait for the pulse to start
    while (gpio_get_level(pin) != state) {
        now = esp_timer_get_time();
        if (now - start > timeout_us) {
            return m5::hal::error::error_t::TIMEOUT_ERROR;
        }
    }

    auto pulse_start = esp_timer_get_time();

    // Wait for the pulse to end
    while (gpio_get_level(pin) == state) {
        now = esp_timer_get_time();
        if (now - pulse_start > timeout_us) {
            return m5::hal::error::error_t::TIMEOUT_ERROR;
        }
    }

    auto pulse_end = esp_timer_get_time();
    duration       = static_cast<uint32_t>(pulse_end - pulse_start);
    return m5::hal::error::error_t::OK;
}

AdapterGPIOBase::AdapterGPIOBase(GPIOImpl* impl) : Adapter(Adapter::Type::GPIO, impl)
{
}

}  // namespace unit
}  // namespace m5
