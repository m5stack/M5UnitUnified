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
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && M5_UNIT_UNIFIED_HAS_RMT

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <esp_private/esp_clk.h>
#include <soc/soc_caps.h>      // SOC_RMT_MEM_WORDS_PER_CHANNEL
#include <soc/soc.h>           // PRO_CPU_NUM
#include <soc/gpio_sig_map.h>  // SIG_GPIO_OUT_IDX
#include <esp_rom_gpio.h>

using namespace m5::unit::gpio;

namespace {

rmt_encoder_handle_t copy_encoder{};

rmt_tx_channel_config_t to_rmt_tx_config(const adapter_config_t &cfg, const uint32_t apb_freq_hz)
{
    rmt_tx_channel_config_t out{};
    out.clk_src = RMT_CLK_SRC_DEFAULT;
    out.mem_block_symbols =
        std::max<uint32_t>(SOC_RMT_MEM_WORDS_PER_CHANNEL, cfg.tx.mem_blocks * SOC_RMT_MEM_WORDS_PER_CHANNEL);
    out.resolution_hz     = calculate_rmt_resolution_hz(apb_freq_hz, cfg.tx.tick_ns);
    out.trans_queue_depth = 4;
    out.flags.with_dma    = cfg.tx.with_dma;
    // NOTE: cfg.tx.loop_enabled drives two independent features:
    //   (1) Hardware loopback (io_loop_back flag) - connects TX pin to RX side
    //       internally for single-wire use cases. The field is removed in IDF
    //       v6, so hardware loopback is silently disabled there. No current
    //       unit uses this path.
    //   (2) Software loop transmission (loop_count in to_rmt_transmit_config)
    //       - repeats the same symbol N times. Still works on IDF v6.
    // TODO: split into independent config fields once a single-wire user appears.
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    out.flags.io_loop_back = cfg.tx.loop_enabled;
#endif
    out.flags.invert_out = cfg.tx.invert_signal;
    return out;
}

rmt_rx_channel_config_t to_rmt_rx_config(const adapter_config_t &cfg, const uint32_t apb_freq_hz)
{
    rmt_rx_channel_config_t out{};
    out.clk_src = RMT_CLK_SRC_DEFAULT;
    out.mem_block_symbols =
        std::max<uint32_t>(SOC_RMT_MEM_WORDS_PER_CHANNEL, cfg.rx.mem_blocks * SOC_RMT_MEM_WORDS_PER_CHANNEL);
    out.resolution_hz   = calculate_rmt_resolution_hz(apb_freq_hz, cfg.rx.tick_ns);
    out.flags.with_dma  = cfg.rx.with_dma;
    out.flags.invert_in = cfg.rx.invert_signal;
    return out;
}

rmt_transmit_config_t to_rmt_transmit_config(const adapter_config_t &cfg)
{
    rmt_transmit_config_t out{};
    out.loop_count              = cfg.tx.loop_enabled ? cfg.tx.loop_count : 0;
    out.flags.eot_level         = cfg.tx.idle_output_enabled ? (cfg.tx.idle_level_high ? 1 : 0) : 0;
    out.flags.queue_nonblocking = 1;
    return out;
}

rmt_receive_config_t to_rmt_receive_config(const adapter_config_t &cfg)
{
    rmt_receive_config_t out{};
    if (cfg.rx.filter_enabled && cfg.rx.filter_ticks_threshold) {
        out.signal_range_min_ns = cfg.rx.filter_ticks_threshold * 10000 / cfg.rx.tick_ns;
    }
    if (cfg.rx.idle_ticks_threshold) {
        out.signal_range_max_ns = static_cast<uint32_t>(cfg.rx.idle_ticks_threshold) * cfg.rx.tick_ns;
    }
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    out.flags.en_partial_rx = 0;
#endif
    return out;
}

void dump_rmt_config(const rmt_tx_channel_config_t &cfg)
{
    M5_LIB_LOGI("=== TX ===");
    M5_LIB_LOGI("gpio_num          : %d", static_cast<int>(cfg.gpio_num));
    M5_LIB_LOGI("clk_src           : %d", cfg.clk_src);
    M5_LIB_LOGI("resolution_hz     : %u", cfg.resolution_hz);
    M5_LIB_LOGI("mem_block_symbols : %zu", cfg.mem_block_symbols);
    M5_LIB_LOGI("trans_queue_depth : %zu", cfg.trans_queue_depth);
    M5_LIB_LOGI("intr_priority     : %d", cfg.intr_priority);
    M5_LIB_LOGI("invert_out        : %u", cfg.flags.invert_out);
    M5_LIB_LOGI("with_dma          : %u", cfg.flags.with_dma);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    M5_LIB_LOGI("allow_pd          : %u", cfg.flags.allow_pd);
#endif
    M5_LIB_LOGI("SOC_RMT_MEM_WORDS_PER_CHANNEL:%u", SOC_RMT_MEM_WORDS_PER_CHANNEL);
}

void dump_rmt_config(const rmt_rx_channel_config_t &cfg)
{
    M5_LIB_LOGI("=== RX ===");
    M5_LIB_LOGI("gpio_num          : %d", static_cast<int>(cfg.gpio_num));
    M5_LIB_LOGI("clk_src           : %d", cfg.clk_src);
    M5_LIB_LOGI("resolution_hz     : %u", cfg.resolution_hz);
    M5_LIB_LOGI("mem_block_symbols : %zu", cfg.mem_block_symbols);
    M5_LIB_LOGI("intr_priority     : %d", cfg.intr_priority);
    M5_LIB_LOGI("invert_in         : %u", cfg.flags.invert_in);
    M5_LIB_LOGI("with_dma          : %u", cfg.flags.with_dma);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    M5_LIB_LOGI("allow_pd          : %u", cfg.flags.allow_pd);
#endif
    M5_LIB_LOGI("SOC_RMT_MEM_WORDS_PER_CHANNEL:%u", SOC_RMT_MEM_WORDS_PER_CHANNEL);
}

void dump_rmt_config(const rmt_transmit_config_t &cfg)
{
    M5_LIB_LOGI("=== TRANSMIT ===");
    M5_LIB_LOGI("loop_count        : %u", cfg.loop_count);
    M5_LIB_LOGI("eot_level         : %u", cfg.flags.eot_level);
    M5_LIB_LOGI("queue_nonblocking : %u", cfg.flags.queue_nonblocking);
}

void dump_rmt_config(const rmt_receive_config_t &cfg)
{
    M5_LIB_LOGI("=== RECEIVE ===");
    M5_LIB_LOGI("signal_range_min_ns : %u", cfg.signal_range_min_ns);
    M5_LIB_LOGI("signal_range_max_ns : %u", cfg.signal_range_max_ns);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    M5_LIB_LOGI("en_partial_rx       : %u", cfg.flags.en_partial_rx);
#endif
}

#if 0
void dump_symbols(const rmt_symbol_word_t *symbols, const uint32_t symbol_num)
{
    for (uint32_t i = 0; i < symbol_num; ++i) {
        auto d = symbols[i];
        M5_LIB_LOGI("[%02u]:{%u,%u,%u,%u}", i, d.duration0, d.level0, d.duration1, d.level1);
    }
}
#endif

}  // namespace

namespace m5 {
namespace unit {

//
class GPIOImplV2 : public AdapterGPIO::GPIOImpl {
public:
    GPIOImplV2(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase::GPIOImpl(rx_pin, tx_pin)
    {
    }
    void *rmtTxHandle() const override
    {
        return _tx_handle;
    }
    void *rmtRxHandle() const override
    {
        return _rx_handle;
    }

    virtual ~GPIOImplV2()
    {
        if (_tx_handle) {
            rmt_disable(_tx_handle);
            rmt_del_channel(_tx_handle);
        }
        // The receive callback touches only this instance's buffers, so it must stop before they are freed.
        // On dual-core chips the callback may be restarting reception on the other core while disabling,
        // which makes rmt_disable / rmt_del_channel fail: stop it from restarting and retry
        if (_rx_handle) {
            _rx_stopping = true;
            bool deleted{};
            for (int_fast8_t i = 0; i < 10 && !deleted; ++i) {
                rmt_disable(_rx_handle);
                deleted = (rmt_del_channel(_rx_handle) == ESP_OK);
                if (!deleted) {
                    m5::utility::delay(1);
                }
            }
            if (!deleted) {
                // Keep the buffers rather than free them under a live channel
                M5_LIB_LOGE("Failed to delete RMT RX channel, its buffers are left allocated");
                return;
            }
        }
        if (_ring_buf) {
            vRingbufferDelete(_ring_buf);
        }
        if (_rx_buf) {
            heap_caps_free(_rx_buf);
        }
    }

    IRAM_ATTR static bool callbackReceive(rmt_channel_handle_t handle, const rmt_rx_done_event_data_t *edata,
                                          void *user_ctx);

    bool begin(const gpio::adapter_config_t &cfg);
    m5::hal::error::error_t writeWithTransaction(const uint8_t *data, const size_t len, const uint32_t waitMs) override;
    m5::hal::error::error_t readWithTransaction(uint8_t *data, const size_t len) override;

protected:
    rmt_channel_handle_t _rx_handle{}, _tx_handle{};
    rmt_rx_channel_config_t _rx_config{};
    rmt_receive_config_t _receive_config{};
    rmt_tx_channel_config_t _tx_config{};
    rmt_transmit_config_t _transmit_config{};

    uint16_t _rx_buf_len{};

    uint8_t *_rx_buf{};
    RingbufHandle_t _ring_buf{};

    volatile uint32_t _dropped{};  // Frames the receive callback could not store (counted in ISR)
    uint32_t _dropped_reported{};
    volatile bool _rx_stopping{};  // Set before disabling: the callback no longer restarts reception
    volatile bool _rx_stalled{};   // The callback failed to restart reception (restarted on the next read)
};

bool GPIOImplV2::begin(const gpio::adapter_config_t &cfg)
{
    // RMT TX
    if (!_tx_handle && (cfg.mode == gpio::Mode::RmtTX || cfg.mode == gpio::Mode::RmtRXTX)) {
        _tx_config          = to_rmt_tx_config(cfg, esp_clk_apb_freq());
        _tx_config.gpio_num = tx_pin();

        auto err = rmt_new_tx_channel(&_tx_config, &_tx_handle);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_new_tx_channel pin:%d %x", tx_pin(), err);
            return false;
        }
        err = rmt_enable(_tx_handle);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_enable %x", err);
            return false;
        }

        _transmit_config = to_rmt_transmit_config(cfg);

        dump_rmt_config(_tx_config);
        dump_rmt_config(_transmit_config);

        M5_LIB_LOGI("Retrieve RMT(v2) TX %d", tx_pin());
    }

    // RMT RX
    if (!_rx_handle && (cfg.mode == gpio::Mode::RmtRX || cfg.mode == gpio::Mode::RmtRXTX)) {
        _rx_buf = (uint8_t *)heap_caps_malloc(cfg.rx.ring_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
        if (!_rx_buf) {
            M5_LIB_LOGE("Failed to allocate memory %u", cfg.rx.ring_buffer_size);
            return false;
        }

        // A NOSPLIT ringbuffer only accepts items up to half its size (minus an 8-byte header),
        // so size it to hold one full receive buffer
        const size_t ring_size = 2 * (((cfg.rx.ring_buffer_size + 3) & ~3u) + 8);
        _ring_buf              = xRingbufferCreate(ring_size, RINGBUF_TYPE_NOSPLIT);
        if (!_ring_buf) {
            M5_LIB_LOGE("Failed to create ringbuffer");
            return false;
        }
        _rx_buf_len = cfg.rx.ring_buffer_size;

        _rx_config          = to_rmt_rx_config(cfg, esp_clk_apb_freq());
        _rx_config.gpio_num = rx_pin();

        auto err = rmt_new_rx_channel(&_rx_config, &_rx_handle);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_new_rx_channel pin:%d %x", rx_pin(), err);
            dump_rmt_config(_rx_config);
            return false;
        }

        rmt_rx_event_callbacks_t cbs{};
        cbs.on_recv_done = callbackReceive;
        err              = rmt_rx_register_event_callbacks(_rx_handle, &cbs, this);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_rx_register_event_callbacks  %x", err);
            return false;
        }

        // ESP-IDF 5.4+ no longer disables the pin output in rmt_new_rx_channel (the legacy driver did),
        // so a pin left driven as an output (e.g. GPIO13/15 after M5Unified's Capsule detection) would block reception.
        // A single-wire pin shared with TX is left to the TX channel
        if (rx_pin() != tx_pin()) {
            if (GPIO_IS_VALID_OUTPUT_GPIO(rx_pin())) {
                esp_rom_gpio_connect_out_signal(rx_pin(), SIG_GPIO_OUT_IDX, false, false);
            }
            gpio_set_direction(rx_pin(), GPIO_MODE_INPUT);
        }

        err = rmt_enable(_rx_handle);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_enable RX %x", err);
            return false;
        }
        _receive_config = to_rmt_receive_config(cfg);

        dump_rmt_config(_rx_config);
        dump_rmt_config(_receive_config);

        // Kick rmt_receive
        err = rmt_receive(_rx_handle, _rx_buf, _rx_buf_len, &_receive_config);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_receive %x", err);
            return false;
        }
        M5_LIB_LOGI("Retrieve RMT(v2) RX %d", rx_pin());
    }

    if (!copy_encoder) {
        rmt_copy_encoder_config_t enc_cfg{};
        auto err = rmt_new_copy_encoder(&enc_cfg, &copy_encoder);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to rmt_new_copy_encoder %x", err);
            return false;
        }
    }

    return true;
}

m5::hal::error::error_t GPIOImplV2::writeWithTransaction(const uint8_t *data, const size_t len, const uint32_t waitMs)
{
    if (!_tx_handle) {
        M5_LIB_LOGE("Invalid handle");
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }

    // m5::utility::log::dump(data, len, false);
    // dump_symbols((rmt_symbol_word_t*)data, len / sizeof(rmt_symbol_word_t));

    auto err = rmt_transmit(_tx_handle, copy_encoder, data, len, &_transmit_config);
    if (err != ESP_OK) {
        M5_LIB_LOGE("Failed to transmit %d:%s", err, esp_err_to_name(err));
    }
    if (err == ESP_OK && waitMs) {
        err = rmt_tx_wait_all_done(_tx_handle, (waitMs == portMAX_DELAY) ? -1 : waitMs);
        if (err != ESP_OK) {
            M5_LIB_LOGE("Failed to wait %d:%s", err, esp_err_to_name(err));
        }
    }
    return err == ESP_OK ? m5::hal::error::error_t::OK : m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t GPIOImplV2::readWithTransaction(uint8_t *data, const size_t len)
{
    if (!_rx_handle || !_ring_buf) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
    if (!data || len < 4) {
        M5_LIB_LOGE("Invalid arguments %p,%zu", data, len);
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }

    const uint32_t dropped = _dropped;
    if (dropped != _dropped_reported) {
        M5_LIB_LOGW("Ringbuffer full, dropped %u frames", static_cast<unsigned>(dropped - _dropped_reported));
        _dropped_reported = dropped;
    }

    if (_rx_stalled && !_rx_stopping) {
        _rx_stalled    = false;
        const auto err = rmt_receive(_rx_handle, _rx_buf, _rx_buf_len, &_receive_config);
        M5_LIB_LOGW("RMT RX had stopped, restarted:%s", esp_err_to_name(err));
        if (err != ESP_OK) {
            _rx_stalled = true;
        }
    }

    size_t max_len = len - 2;  // Top of 2 bytes is receive length
    size_t rx_size{};
    auto *items = static_cast<uint8_t *>(xRingbufferReceive(_ring_buf, &rx_size, pdMS_TO_TICKS(50)));

    memcpy(data, "\0\0", 2);
    if (items && rx_size) {
        uint16_t rlen = (rx_size > max_len) ? static_cast<uint16_t>(max_len) : static_cast<uint16_t>(rx_size);
        memcpy(data, &rlen, sizeof(rlen));
        memcpy(data + 2, items, rlen);
    }
    if (items) {
        vRingbufferReturnItem(_ring_buf, items);
    }
    return rx_size ? m5::hal::error::error_t::OK : m5::hal::error::error_t::TIMEOUT_ERROR;
}

// Runs in ISR: hand the frame to the reader and restart reception into the same buffer (both calls are ISR-safe).
// Everything stays within this instance, so nothing outlives it once the channel is disabled
IRAM_ATTR bool GPIOImplV2::callbackReceive(rmt_channel_handle_t handle, const rmt_rx_done_event_data_t *edata,
                                           void *user_ctx)
{
    auto *me = static_cast<GPIOImplV2 *>(user_ctx);
    if (me->_rx_stopping) {
        return false;
    }
    BaseType_t high_task_wakeup{pdFALSE};
    const size_t len = edata->num_symbols * sizeof(rmt_symbol_word_t);
    if (len && xRingbufferSendFromISR(me->_ring_buf, edata->received_symbols, len, &high_task_wakeup) != pdTRUE) {
        me->_dropped = me->_dropped + 1;
    }
    if (rmt_receive(handle, me->_rx_buf, me->_rx_buf_len, &me->_receive_config) != ESP_OK) {
        me->_rx_stalled = true;
    }
    return high_task_wakeup == pdTRUE;
}

//
AdapterGPIO::AdapterGPIO(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase(new GPIOImplV2(rx_pin, tx_pin))
{
}

}  // namespace unit
}  // namespace m5
#endif

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && !M5_UNIT_UNIFIED_HAS_RMT
namespace m5 {
namespace unit {

// No RMT on this SoC (e.g. ESP32-C61): digital I/O, ADC and pulseIn work, RMT modes fail in begin()
class GPIOImplNoRmt : public AdapterGPIOBase::GPIOImpl {
public:
    GPIOImplNoRmt(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase::GPIOImpl(rx_pin, tx_pin)
    {
    }
    bool begin(const gpio::adapter_config_t &) override
    {
        M5_LIB_LOGE("RMT is not supported on this SoC");
        return false;
    }
};

AdapterGPIO::AdapterGPIO(const int8_t rx_pin, const int8_t tx_pin) : AdapterGPIOBase(new GPIOImplNoRmt(rx_pin, tx_pin))
{
}

}  // namespace unit
}  // namespace m5
#endif

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2) && defined(RMT_CHANNEL_0)
#error "RMT v1 is mixed in with RMT v2 even though RMT v2 is used"
#endif

#if 0
ESP32 board!

SEND V1

|Device|V1|V2|
|---|---|---|
|Core|OK|OK|
|Core2|OK|OK|
|Dial|OK|OK|
|CoreS3|OK|OK|
|NanoC6|---|OK|
|AtomS3R|OK|OK|
|AtomS3||OK|OK|
|CPlus|OK |OK |
|CPlus2|OK | OK|
|StampS3| OK| OK|
|DinMeter|OK|OK|
|CoreINK|OK|OK|
|Paper|OK |OK |
|Fire|OK |OK |
|Matrix|OK |OK |

#endif

#if 0
ESP32
enumerator RMT_CLK_SRC_APB
Select APB as the source clock
enumerator RMT_CLK_SRC_REF_TICK
Select REF_TICK as the source clock
enumerator RMT_CLK_SRC_DEFAULT
Select APB as the default choice


ESP32S3
enumerator RMT_CLK_SRC_APB
Select APB as the source clock
enumerator RMT_CLK_SRC_RC_FAST
Select RC_FAST as the source clock
enumerator RMT_CLK_SRC_XTAL
Select XTAL as the source clock
enumerator RMT_CLK_SRC_DEFAULT
Select APB as the default choice


ESP32C6
enumerator RMT_CLK_SRC_PLL_F80M
Select PLL_F80M as the source clock
enumerator RMT_CLK_SRC_RC_FAST
Select RC_FAST as the source clock
enumerator RMT_CLK_SRC_XTAL
Select XTAL as the source clock
enumerator RMT_CLK_SRC_DEFAULT
Select PLL_F80M as the default choice

#endif
