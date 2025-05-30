/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_gpio_v2.cpp
  @brief Adapters to treat M5HAL and GPIO in the same way using RMT v2
  @note  Currently handles GPIO directly, but will handle via M5HAL in the future
*/
#include "adapter_gpio_v2.hpp"
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)

#include <esp_private/esp_clk.h>
using namespace m5::unit::gpio;

namespace {

rmt_encoder_handle_t copy_encoder{};

rmt_tx_channel_config_t to_rmt_tx_config(const adapter_config_t& cfg, const uint32_t apb_freq_hz)
{
    rmt_tx_channel_config_t out{};
    out.clk_src           = RMT_CLK_SRC_DEFAULT;
    out.mem_block_symbols = cfg.tx.mem_blocks * 16;
    out.resolution_hz     = calculate_rmt_resolution_hz(apb_freq_hz, cfg.tx.tick_ns);
    out.trans_queue_depth = 4;
    out.flags.with_dma    = cfg.tx.with_dma;
    return out;
}

rmt_rx_channel_config_t to_rmt_rx_config(const adapter_config_t& cfg, const uint32_t apb_freq_hz)
{
    rmt_rx_channel_config_t out{};
    return out;
}

}  // namespace

namespace m5 {
namespace unit {

//
class GPIOImplV2 : public AdapterGPIO::GPIOImpl {
public:
    GPIOImplV2(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase::GPIOImpl(rx_pin, tx_pin)
    {
    }

    bool begin(const gpio::adapter_config_t& cfg)
    {
        // RMT TX
        if (!_tx_handle && cfg.mode == gpio::Mode::RmtTX || cfg.mode == gpio::Mode::RmtRXTX) {
            _tx_config          = to_rmt_tx_config(cfg, esp_clk_apb_freq());
            _tx_config.gpio_num = tx_pin();

            auto err = rmt_new_tx_channel(&_tx_config, &_tx_handle);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to rmt_new_tx_channel pin:%d", tx_pin());
                return false;
            }
            err = rmt_enable(_tx_handle);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to rmt_enable");
                return false;
            }
        }
        // RMT RX
        if (!_rx_handle && cfg.mode == gpio::Mode::RmtRX || cfg.mode == gpio::Mode::RmtRXTX) {
            // TODO
        }

        if (!copy_encoder) {
            rmt_copy_encoder_config_t enc_cfg{};
            auto err = rmt_new_copy_encoder(&enc_cfg, &copy_encoder);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to rmt_new_copy_encoder");
                return false;
            }
        }
        return true;
    }

    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t waitMs) override
    {
        rmt_transmit_config_t tx_config = {};
        auto err = rmt_transmit(_tx_handle, copy_encoder, (gpio::m5_rmt_item_t*)data, len * sizeof(gpio::m5_rmt_item_t),
                                &tx_config);
        if (err == ESP_OK && waitMs) {
            err = rmt_tx_wait_all_done(_tx_handle, waitMs);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to wait %d:%s", err, esp_err_to_name(err));
            }
        }

        return err == ESP_OK ? m5::hal::error::error_t::OK : m5::hal::error::error_t::UNKNOWN_ERROR;
    }

protected:
    rmt_channel_handle_t _rx_handle{}, _tx_handle{};
    rmt_rx_channel_config_t _rx_config{};
    rmt_tx_channel_config_t _tx_config{};
};

AdapterGPIO::AdapterGPIO(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase(new GPIOImplV2(rx_pin, tx_pin))
{
}

}  // namespace unit
}  // namespace m5
#endif
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && defined(RMT_CHANNEL_0)
#error "RMT v1 is mixed in with RMT v2 even though RMT v2 is used"
#endif
