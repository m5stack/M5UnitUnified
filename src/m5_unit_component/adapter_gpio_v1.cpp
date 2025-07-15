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

// #include <esp_clk.h>
#include <esp32/clk.h>
using namespace m5::unit::gpio;

namespace {

uint32_t using_rmt_channel_bits{};

rmt_channel_t retrieve_available_rmt_channel(const int8_t first = 0)
{
    for (int_fast8_t ch = first; ch < RMT_CHANNEL_MAX; ++ch) {
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
    out.rmt_mode          = RMT_MODE_TX;
    out.mem_block_num     = std::min<uint8_t>(cfg.tx.mem_blocks, 8u);
    out.clk_div           = calculate_rmt_clk_div(apb_freq_hz, cfg.tx.tick_ns);
    out.tx_config.loop_en = cfg.tx.loop_enabled;
    //    out.tx_config.carrier_en     = cfg.tx.carrier_enabled;
    out.tx_config.idle_output_en = cfg.tx.idle_output_enabled;
    out.tx_config.idle_level     = cfg.tx.idle_level_high ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
    return out;
}

rmt_config_t to_rmt_config_rx(const adapter_config_t& cfg, const uint32_t apb_freq_hz)
{
    rmt_config_t out{};
    out.rmt_mode            = RMT_MODE_RX;
    out.mem_block_num       = std::min<uint8_t>(cfg.rx.mem_blocks, 8u);
    out.clk_div             = calculate_rmt_clk_div(apb_freq_hz, cfg.rx.tick_ns);
    out.rx_config.filter_en = cfg.rx.filter_enabled;
    out.rx_config.filter_ticks_thresh =
        std::min<uint8_t>(static_cast<uint32_t>(cfg.rx.filter_ticks_threshold) * 1000 / cfg.rx.tick_ns, 255U);
    out.rx_config.idle_threshold =
        static_cast<uint16_t>(static_cast<uint32_t>(cfg.rx.idle_ticks_threshold) * 1000 / cfg.rx.tick_ns);
    return out;
}

void dump_rmt_config(const rmt_config_t& cfg, const char* title = "RMT CONFIG")
{
    M5_LIB_LOGI("=== %s ===", title);
    M5_LIB_LOGI("rmt_mode       : %s", cfg.rmt_mode == RMT_MODE_TX ? "TX" : "RX");
    M5_LIB_LOGI("channel        : %d", static_cast<int>(cfg.channel));
    M5_LIB_LOGI("gpio_num       : %d", static_cast<int>(cfg.gpio_num));
    M5_LIB_LOGI("mem_block_num  : %d", cfg.mem_block_num);
    M5_LIB_LOGI("clk_div        : %d", cfg.clk_div);

    if (cfg.rmt_mode == RMT_MODE_TX) {
        M5_LIB_LOGI("TX CONFIG:");
        M5_LIB_LOGI("  loop_en         : %s", cfg.tx_config.loop_en ? "true" : "false");
        M5_LIB_LOGI("  carrier_en      : %s", cfg.tx_config.carrier_en ? "true" : "false");
        M5_LIB_LOGI("  idle_output_en  : %s", cfg.tx_config.idle_output_en ? "true" : "false");
        M5_LIB_LOGI("  idle_level      : %d", cfg.tx_config.idle_level);
        M5_LIB_LOGI("  carrier_freq_hz : %d", cfg.tx_config.carrier_freq_hz);
        M5_LIB_LOGI("  carrier_duty    : %d%%", cfg.tx_config.carrier_duty_percent);
        M5_LIB_LOGI("  carrier_level   : %d", cfg.tx_config.carrier_level);
    } else {
        M5_LIB_LOGI("RX CONFIG:");
        M5_LIB_LOGI("  filter_en       : %s", cfg.rx_config.filter_en ? "true" : "false");
        M5_LIB_LOGI("  filter_thresh   : %d", cfg.rx_config.filter_ticks_thresh);
        M5_LIB_LOGI("  idle_threshold  : %d", cfg.rx_config.idle_threshold);
    }
}

#if 0
void dump_items(const rmt_item32_t* items, const uint32_t item_num)
{
    for (uint32_t i = 0; i < item_num; ++i) {
        auto d = items[i];
        M5_LIB_LOGI("[%02u]:{%u,%u,%u,%u}", i, d.duration0, d.level0, d.duration1, d.level1);
    }
}
#endif

}  // namespace

namespace m5 {
namespace unit {

class GPIOImplV1 : public AdapterGPIOBase::GPIOImpl {
public:
    explicit GPIOImplV1(const int8_t rx_pin = -1, const int8_t tx_pin = -1) : AdapterGPIOBase::GPIOImpl(rx_pin, tx_pin)
    {
        _rx_config.channel = RMT_CHANNEL_MAX;
        _tx_config.channel = RMT_CHANNEL_MAX;
    }
    virtual ~GPIOImplV1()
    {
        if (_tx_config.channel != RMT_CHANNEL_MAX) {
            rmt_tx_stop(_tx_config.channel);
            rmt_driver_uninstall(_tx_config.channel);
            clear_use_rmt_channel(_tx_config.channel);
        }
        if (_rx_config.channel != RMT_CHANNEL_MAX) {
            rmt_rx_stop(_rx_config.channel);
            rmt_driver_uninstall(_rx_config.channel);
            clear_use_rmt_channel(_rx_config.channel);
        }
    }

    virtual bool begin(const gpio::adapter_config_t& cfg) override
    {
        _adapter_cfg = cfg;

        // RMT TX
        if (_tx_config.channel == RMT_CHANNEL_MAX &&
            (cfg.mode == gpio::Mode::RmtTX || cfg.mode == gpio::Mode::RmtRXTX)) {
            rmt_channel_t ch = retrieve_available_rmt_channel();
            if (ch >= RMT_CHANNEL_MAX) {
                M5_LIB_LOGE("RMT(v1) No room on TX channel");
                return false;
            }
            _tx_config          = to_rmt_config_tx(cfg, esp_clk_apb_freq());
            _tx_config.channel  = ch;
            _tx_config.gpio_num = tx_pin();

            dump_rmt_config(_tx_config, "TX");

            auto err = rmt_driver_install(_tx_config.channel, 0, 0);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to install TX %d:%s", err, esp_err_to_name(err));
                return false;
            }

            err = rmt_config(&_tx_config);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to config TX %d:%s", err, esp_err_to_name(err));
                return false;
            }

            if (_adapter_cfg.tx.invert_signal) {
                gpio_matrix_out(_tx_config.gpio_num, _tx_config.channel + RMT_SIG_OUT0_IDX, true, false);
            }

            declrare_use_rmt_channel(ch);
            M5_LIB_LOGI("Retrive RMT(v1) TX %d/%u", tx_pin(), ch);
        }
        // RMT RX
        if (_rx_config.channel == RMT_CHANNEL_MAX &&
            (cfg.mode == gpio::Mode::RmtRX || cfg.mode == gpio::Mode::RmtRXTX)) {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
            int8_t first = 4;  // RX channel 4 - 7
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
            int8_t first = 2;  // RX channel 2 - 3
#else
            int8_t first = 0;
#endif
            rmt_channel_t ch = retrieve_available_rmt_channel(first);
            if (ch >= RMT_CHANNEL_MAX) {
                M5_LIB_LOGE("RMT(v1) No room on RX channel");
                return false;
            }

            _rx_config          = to_rmt_config_rx(cfg, esp_clk_apb_freq());
            _rx_config.channel  = ch;
            _rx_config.gpio_num = rx_pin();

            dump_rmt_config(_rx_config, "RX");

            auto err = rmt_driver_install(_rx_config.channel, cfg.rx.ring_buffer_size, 0);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to install RX %d:%s", err, esp_err_to_name(err));
                return false;
            }

            err = rmt_config(&_rx_config);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to config RX %d:%s", err, esp_err_to_name(err));
                return false;
            }

            if (_adapter_cfg.rx.invert_signal) {
                gpio_matrix_in(_rx_config.gpio_num, _rx_config.channel + RMT_SIG_IN0_IDX, true);
            }

            declrare_use_rmt_channel(ch);
            M5_LIB_LOGI("Retrieve RMT(v1) RX %d/%u", rx_pin(), ch);

            if (rmt_rx_start(_rx_config.channel, true) != ESP_OK) {
                M5_LIB_LOGE("Failed to start RX");
                return false;
            }

            // Discard garbage
            size_t rx_size{};
            RingbufHandle_t rb{};
            rmt_get_ringbuf_handle(_rx_config.channel, &rb);
            if (rb) {
                rmt_item32_t* items = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, 10 / portTICK_PERIOD_MS);
                if (items) {
                    vRingbufferReturnItem(rb, items);
                }
            }
        }
        return true;
    }

    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t waitMs) override
    {
        if (_tx_config.channel == RMT_CHANNEL_MAX) {
            M5_LIB_LOGE("Invalid channel");
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        // m5::utility::log::dump(data, len, false);
        // dump_items((rmt_item32_t*)data, len / sizeof(rmt_item32_t));

        auto err = rmt_write_items(_tx_config.channel, (gpio::m5_rmt_item_t*)data, len / sizeof(rmt_item32_t), false);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to write %d:%s", err, esp_err_to_name(err));
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }
        if (err == ESP_OK && waitMs) {
            // M5_LIB_LOGE(">>> wait_tx_done %d,%u", _tx_config.channel, waitMs);
            err = rmt_wait_tx_done(_tx_config.channel, waitMs);
            if (err != ESP_OK) {
                M5_LIB_LOGE("Failed to wait %d:%s", err, esp_err_to_name(err));
            }
        }
        return err == ESP_OK ? m5::hal::error::error_t::OK : m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len)
    {
        if (_rx_config.channel == RMT_CHANNEL_MAX) {
            M5_LIB_LOGE("Invalid channel");
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        if (len < 4) {
            M5_LIB_LOGE("length too small %zu", len);
            return m5::hal::error::error_t::INVALID_ARGUMENT;
        }

        RingbufHandle_t rb{};
        if (rmt_get_ringbuf_handle(_rx_config.channel, &rb) != ESP_OK || rb == nullptr) {
            M5_LIB_LOGE("Failed to get RX ringbuffer");
            return m5::hal::error::error_t::UNKNOWN_ERROR;
        }

        size_t max_len = len - 2;  // Top of 2bytes is receive length
        size_t rx_size{};
        rmt_item32_t* items = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, pdMS_TO_TICKS(50));

        // dump_items(items, rx_size / sizeof(rmt_item32_t));
        *(uint16_t*)data = 0;
        if (items && rx_size) {
            uint16_t rlen    = std::min<uint16_t>(rx_size, max_len);
            *(uint16_t*)data = rlen;
            memcpy(data + 2, items, rlen);
        }
        if (items) {
            vRingbufferReturnItem(rb, items);
        }
        return rx_size ? m5::hal::error::error_t::OK : m5::hal::error::error_t::TIMEOUT_ERROR;
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
