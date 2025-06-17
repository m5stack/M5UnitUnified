/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_gpio_v1.cpp
  @brief Adapters to treat M5HAL and GPIO in the same way using RNT v1
  @note  Currently handles GPIO directly, but will handle via M5HAL in the future
*/
#include "adapter_gpio_v1.hpp"

#if !defined(M5_UNIT_UNIFIED_USING_RMT_V2)

#include <esp_clk.h>
using namespace m5::unit::gpio;

namespace {

uint32_t using_rmt_channel_bits{};

rmt_channel_t retrieve_available_rmt_channel()
{
    for (int_fast8_t ch = 0; ch < RMT_CHANNEL_MAX; ++ch) {
        if (((1U << ch) & using_rmt_channel_bits) == 0) {
            return (rmt_channel_t)ch;
        }
    }
    return RMT_CHANNEL_MAX;
}

bool declrare_use_rmt_channel(const int ch)
{
    if (ch >= 0 && ch < RMT_CHANNEL_MAX && ((1U << ch) & using_rmt_channel_bits) == 0) {
        using_rmt_channel_bits |= (1U << ch);
        return true;
    }
    return false;
}

void clear_use_rmt_channel(const int ch)
{
    if (ch >= 0 && ch < RMT_CHANNEL_MAX) {
        using_rmt_channel_bits &= ~(1U << ch);
    }
}

rmt_config_t to_rmt_config_tx(const adapter_config_t& cfg, const uint32_t apb_freq_hz)
{
    rmt_config_t out{};
    out.rmt_mode      = RMT_MODE_TX;
    out.mem_block_num = cfg.tx.mem_blocks;

    out.clk_div                  = calculate_rmt_clk_div(apb_freq_hz, cfg.tx.tick_ns);
    out.tx_config.carrier_en     = false;
    out.tx_config.idle_output_en = cfg.tx.idle_output;
    out.tx_config.idle_level     = cfg.tx.idle_level_high ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
    out.tx_config.loop_en        = cfg.tx.loop_enabled;
    return out;
}

#if 0
rmt_config_t to_rmt_config_rx(const adapter_config_t& cfg, const uint32_t apb_freq_hz)
{
    rmt_config_t out{};
    out.rmt_mode                      = RMT_MODE_RX;
    out.mem_block_num                 = cfg.tx.mem_blocks;
    out.clk_div                       = calculate_rmt_clk_div(apb_freq_hz, cfg.rx.tick_ns);
    out.rx_config.filter_en           = true;
    out.rx_config.filter_ticks_thresh = 30;
    out.rx_config.idle_threshold      = 300;
    return out;
}
#endif
}  // namespace

namespace m5 {
namespace unit {

class GPIOImplV1 : public AdapterGPIOBase::GPIOImpl {
public:
    GPIOImplV1() : AdapterGPIOBase::GPIOImpl(-1, -1)
    {
    }
    GPIOImplV1(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase::GPIOImpl(rx_pin, tx_pin)
    {
    }
    virtual ~GPIOImplV1()
    {
        rmt_tx_stop(_tx_config.channel);
        rmt_driver_uninstall(_tx_config.channel);
        clear_use_rmt_channel(_tx_config.channel);

        rmt_rx_stop(_rx_config.channel);
        rmt_driver_uninstall(_rx_config.channel);
        clear_use_rmt_channel(_rx_config.channel);
    }

    virtual bool begin(const gpio::adapter_config_t& cfg) override
    {
        _adapter_cfg = cfg;

        if (_tx_config.clk_div || _rx_config.clk_div) {
            M5_LIB_LOGD("Already begun");
            return true;
        }

        // RMT TX
        if (cfg.mode == gpio::Mode::RmtTX || cfg.mode == gpio::Mode::RmtRXTX) {
            rmt_channel_t ch = retrieve_available_rmt_channel();
            if (ch >= RMT_CHANNEL_MAX) {
                M5_LIB_LOGE("RMT(v1) No room on channel");
                return false;
            }
            declrare_use_rmt_channel(ch);
            M5_LIB_LOGI("Retrive RMT(v1) %u", ch);

            _tx_config          = to_rmt_config_tx(cfg, esp_clk_apb_freq());
            _tx_config.channel  = ch;
            _tx_config.gpio_num = tx_pin();

            auto err = rmt_config(&_tx_config);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to configurate %d:%s", err, esp_err_to_name(err));
                return false;
            }
            err = rmt_driver_install(_tx_config.channel, 0, 0);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to install %d:%s", err, esp_err_to_name(err));
                return false;
            }
        }
        // RMT RX
        if (cfg.mode == gpio::Mode::RmtRX || cfg.mode == gpio::Mode::RmtRXTX) {
            // TODO
        }

        return true;
    }

    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t waitMs) override
    {
        if (_adapter_cfg.mode == Mode::RmtTX || _adapter_cfg.mode == Mode::RmtRXTX) {
            auto err = rmt_write_items(_tx_config.channel, (gpio::m5_rmt_item_t*)data, len, true);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to write %d:%s", err, esp_err_to_name(err));
                return m5::hal::error::error_t::UNKNOWN_ERROR;
            }
            if (err == ESP_OK && waitMs) {
                err = rmt_wait_tx_done(_tx_config.channel, pdMS_TO_TICKS(50));
                if (err != ESP_OK) {
                    M5_LIB_LOGE("Failed to wait %d:%s", err, esp_err_to_name(err));
                }
            }
            return err == ESP_OK ? m5::hal::error::error_t::OK : m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        M5_LIB_LOGE("Failed invalid config");
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    void copy_from(GPIOImplV1* ptr)
    {
        _rx_pin      = ptr->_rx_pin;
        _tx_pin      = ptr->_tx_pin;
        _adapter_cfg = ptr->_adapter_cfg;
        _rx_config   = ptr->_rx_config;
        _tx_config   = ptr->_tx_config;
    }

protected:
    rmt_config_t _rx_config{}, _tx_config{};
};

AdapterGPIO::AdapterGPIO(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase(new GPIOImplV1(rx_pin, tx_pin))
{
}

//
}  // namespace unit
}  // namespace m5
#endif
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && defined(RMT_CHANNEL_0)
#error "RMT v1 is mixed in with RMT v2 even though RMT v2 is used"
#endif
