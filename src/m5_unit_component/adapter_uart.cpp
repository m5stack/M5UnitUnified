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

void AdapterUART::ESPIDFImpl::flush()
{
    if (!uart_is_driver_installed(_uart_num)) {
        return;
    }
    uart_wait_tx_done(_uart_num, pdMS_TO_TICKS(_timeout_ms));
}

void AdapterUART::ESPIDFImpl::flushRX()
{
    if (!uart_is_driver_installed(_uart_num)) {
        return;
    }
    uart_flush_input(_uart_num);
}

void AdapterUART::ESPIDFImpl::setTimeout(const uint32_t ms)
{
    _timeout_ms = ms;
}

m5::hal::error::error_t AdapterUART::ESPIDFImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    if (!uart_is_driver_installed(_uart_num) || data == nullptr) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
    int got = uart_read_bytes(_uart_num, data, len, pdMS_TO_TICKS(_timeout_ms));
    if (got < 0 || static_cast<size_t>(got) != len) {
        return m5::hal::error::error_t::TIMEOUT_ERROR;
    }
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterUART::ESPIDFImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                      const uint32_t /* stop */)
{
    if (!uart_is_driver_installed(_uart_num) || data == nullptr) {
        return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
    int written = uart_write_bytes(_uart_num, reinterpret_cast<const char*>(data), len);
    if (written < 0 || static_cast<size_t>(written) != len) {
        return m5::hal::error::error_t::TIMEOUT_ERROR;
    }
    return m5::hal::error::error_t::OK;
}

AdapterUART::AdapterUART(const uart_port_t uart_num)
    : Adapter(Adapter::Type::UART, new AdapterUART::ESPIDFImpl(uart_num))
{
    assert(_impl);
}

#endif

}  // namespace unit
}  // namespace m5
