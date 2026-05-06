/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_uart.cpp
  @brief Adapters to treat M5HAL and UART in the same way using Serial
  @note  Currently handles Serial directly, but will handle via M5HAL in the future
*/
#include "adapter_uart.hpp"
#if defined(ARDUINO)
#include <HardwareSerial.h>
// #include <SoftwareSerial.h>
#endif
#if defined(ESP_PLATFORM)
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_idf_version.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <cassert>

namespace m5 {
namespace unit {

#if defined(ARDUINO)

AdapterUART::SerialImpl::SerialImpl(HardwareSerial& serial) : AdapterUART::UARTImpl(), _serial(&serial)
{
}

void AdapterUART::SerialImpl::flush()
{
    _serial->flush();
}

void AdapterUART::SerialImpl::flushRX()
{
    while (_serial->available()) {
        (void)_serial->read();  // Discard
    }
}

void AdapterUART::SerialImpl::setTimeout(const uint32_t ms)
{
    _serial->setTimeout(ms);
}

m5::hal::error::error_t AdapterUART::SerialImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    return (_serial->readBytes(data, len) == len) ? m5::hal::error::error_t::OK
                                                  : m5::hal::error::error_t::TIMEOUT_ERROR;
}

m5::hal::error::error_t AdapterUART::SerialImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                      const uint32_t /* unused */)
{
    return (_serial->write(data, len) == len) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::TIMEOUT_ERROR;
}

AdapterUART::AdapterUART(HardwareSerial& serial) : Adapter(Adapter::Type::UART, new AdapterUART::SerialImpl(serial))
{
    assert(_impl);
}
#endif

#if defined(ESP_PLATFORM)

AdapterUART::ESPIDFImpl::ESPIDFImpl(const Config& cfg) : AdapterUART::UARTImpl(), _cfg(cfg)
{
    uart_config_t uart_cfg{};
    uart_cfg.baud_rate           = _cfg.baud_rate;
    uart_cfg.data_bits           = _cfg.data_bits;
    uart_cfg.parity              = _cfg.parity;
    uart_cfg.stop_bits           = _cfg.stop_bits;
    uart_cfg.flow_ctrl           = _cfg.flow_ctrl;
    uart_cfg.rx_flow_ctrl_thresh = 0;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    uart_cfg.source_clk = UART_SCLK_DEFAULT;
#else
    uart_cfg.source_clk = UART_SCLK_APB;
#endif

    esp_err_t err = uart_driver_install(_cfg.uart_num, _cfg.buf_size, 0, 0, nullptr, 0);
    if (err != ESP_OK) {
        M5_LIB_LOGE("uart_driver_install failed: %d", err);
        return;
    }
    err = uart_param_config(_cfg.uart_num, &uart_cfg);
    if (err != ESP_OK) {
        M5_LIB_LOGE("uart_param_config failed: %d", err);
        uart_driver_delete(_cfg.uart_num);
        return;
    }
    err = uart_set_pin(_cfg.uart_num, _cfg.tx_pin, _cfg.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        M5_LIB_LOGE("uart_set_pin failed: %d", err);
        uart_driver_delete(_cfg.uart_num);
        return;
    }
    _installed = true;
}

AdapterUART::ESPIDFImpl::~ESPIDFImpl()
{
    if (_installed) {
        uart_driver_delete(_cfg.uart_num);
        _installed = false;
    }
}

void AdapterUART::ESPIDFImpl::flush()
{
    if (!_installed) {
        return;
    }
    uart_wait_tx_done(_cfg.uart_num, pdMS_TO_TICKS(_timeout_ms));
}

void AdapterUART::ESPIDFImpl::flushRX()
{
    if (!_installed) {
        return;
    }
    uart_flush_input(_cfg.uart_num);
}

void AdapterUART::ESPIDFImpl::setTimeout(const uint32_t ms)
{
    _timeout_ms = ms;
}

m5::hal::error::error_t AdapterUART::ESPIDFImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    if (!_installed || data == nullptr) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
    int got = uart_read_bytes(_cfg.uart_num, data, len, pdMS_TO_TICKS(_timeout_ms));
    if (got < 0 || static_cast<size_t>(got) != len) {
        return m5::hal::error::error_t::TIMEOUT_ERROR;
    }
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterUART::ESPIDFImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                      const uint32_t /* stop */)
{
    if (!_installed || data == nullptr) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
    int written = uart_write_bytes(_cfg.uart_num, reinterpret_cast<const char*>(data), len);
    if (written < 0 || static_cast<size_t>(written) != len) {
        return m5::hal::error::error_t::TIMEOUT_ERROR;
    }
    return m5::hal::error::error_t::OK;
}

AdapterUART::AdapterUART(const ESPIDFImpl::Config& cfg) : Adapter(Adapter::Type::UART, new AdapterUART::ESPIDFImpl(cfg))
{
    assert(_impl);
}

AdapterUART::AdapterUART(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, int buf_size)
    : Adapter(Adapter::Type::UART, nullptr)
{
    ESPIDFImpl::Config cfg{};
    cfg.uart_num  = uart_num;
    cfg.baud_rate = baud_rate;
    cfg.rx_pin    = rx_pin;
    cfg.tx_pin    = tx_pin;
    cfg.buf_size  = buf_size;
    _impl.reset(new AdapterUART::ESPIDFImpl(cfg));
    assert(_impl);
}

#endif

}  // namespace unit
}  // namespace m5
