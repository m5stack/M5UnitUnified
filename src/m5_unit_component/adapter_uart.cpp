/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_uart.cpp
  @brief Adapters to treat M5HAL and UART in the same way using Searial
  @note  Currently handles Serial directly, but will handle via M5HAL in the future
*/
#include "adapter_uart.hpp"
#if defined(ARDUINO)
#include <HardwareSerial.h>
// #include <SoftwareSerial.h>
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
        _serial->read();  // Discard
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
                                                                      const uint32_t /* unuse */)
{
    return (_serial->write(data, len) == len) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::TIMEOUT_ERROR;
}

AdapterUART::AdapterUART(HardwareSerial& serial) : Adapter(Adapter::Type::UART, new AdapterUART::SerialImpl(serial))
{
    assert(_impl);
}
#endif

}  // namespace unit
}  // namespace m5
