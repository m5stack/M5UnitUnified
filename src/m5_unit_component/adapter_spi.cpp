/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_spi.cpp
  @brief Adapters to treat M5HAL and SPI in the same way using SPI
  @note  Currently handles SPI directly, but will handle via M5HAL in the future
*/
#include "adapter_spi.hpp"
#if defined(ESP_PLATFORM)
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <cassert>

namespace m5 {
namespace unit {

#if defined(ARDUINO)
uint32_t AdapterSPI::SPIClassImpl::transaction_count{};

AdapterSPI::SPIClassImpl::SPIClassImpl(SPIClass& spi, const SPISettings& settings, const gpio_num_t cs)
    : AdapterSPI::SPIImpl(cs), _spi(&spi), _settings{settings}
{
    if (_cs != GPIO_NUM_NC) {
        gpio_set_direction(_cs, GPIO_MODE_OUTPUT);
        gpio_set_level(_cs, 1);  // Idle high
    }
}

void AdapterSPI::SPIClassImpl::beginTransaction()
{
    if (transaction_count++ == 0) {
        _spi->beginTransaction(_settings);
        if (cs_pin() != GPIO_NUM_NC) {
            gpio_set_level(cs_pin(), 0);
        }
    } else {
        M5_LIB_LOGE("Don't nest!");
    }
}

void AdapterSPI::SPIClassImpl::endTransaction()
{
    if (transaction_count && --transaction_count == 0) {
        if (cs_pin() != GPIO_NUM_NC) {
            gpio_set_level(cs_pin(), 1);
        }
        _spi->endTransaction();
    } else {
        M5_LIB_LOGE("Don't nest!");
    }
}

m5::hal::error::error_t AdapterSPI::SPIClassImpl::readWithTransaction(uint8_t* data, const size_t len)
{
#if 0
    for (size_t i = 0; i < len; ++i) {
        data[i] = _spi->transfer(0x00 /*dummy*/);
        M5_LIB_LOGE("R[%2u]:%02X", i, data[i]);
    }
#else
    _spi->transferBytes(nullptr, data, len);
#endif
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterSPI::SPIClassImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                       const uint32_t /* unused */)
{
#if 0
    for (size_t i = 0; i < len; ++i) {
        M5_LIB_LOGE("W[%2u]:%02X", i, data[i]);
        _spi->transfer(data[i]);
    }
#else
    _spi->transferBytes(data, nullptr, len);
#endif
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterSPI::SPIClassImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                       const size_t len, const uint32_t)
{
    auto ret = writeWithTransaction(&reg, 1, 0);
    if (data && len && ret == m5::hal::error::error_t::OK) {
        ret = writeWithTransaction(data, len, 0);
    }
    return ret;
}

m5::hal::error::error_t AdapterSPI::SPIClassImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                       const size_t len, const uint32_t)
{
    m5::types::big_uint16_t r(reg);
    auto ret = writeWithTransaction(r.data(), r.size(), 0);
    if (data && len && ret == m5::hal::error::error_t::OK) {
        ret = writeWithTransaction(data, len, 0);
    }
    return ret;
}

AdapterSPI::AdapterSPI(SPIClass& spi, const SPISettings& settings, const gpio_num_t cs)
    : Adapter(Adapter::Type::SPI, new AdapterSPI::SPIClassImpl(spi, settings, cs))
{
    assert(_impl);
}
#endif

#if defined(ESP_PLATFORM)
namespace {
m5::hal::error::error_t to_spi_error(const esp_err_t err)
{
    switch (err) {
        case ESP_OK:
            return m5::hal::error::error_t::OK;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_STATE:
            return m5::hal::error::error_t::INVALID_ARGUMENT;
        case ESP_ERR_TIMEOUT:
            return m5::hal::error::error_t::TIMEOUT_ERROR;
        default:
            return m5::hal::error::error_t::UNKNOWN_ERROR;
    }
}
}  // namespace

AdapterSPI::ESPIDFImpl::ESPIDFImpl(spi_device_handle_t handle, const gpio_num_t cs)
    : AdapterSPI::SPIImpl(), _handle(handle), _cs(cs)
{
    if (_cs != GPIO_NUM_NC) {
        gpio_set_direction(_cs, GPIO_MODE_OUTPUT);
        gpio_set_level(_cs, 1);  // Idle high
    }
}

void AdapterSPI::ESPIDFImpl::beginTransaction()
{
    if (_in_transaction) {
        M5_LIB_LOGE("Don't nest!");
        return;
    }
    _in_transaction = true;
    spi_device_acquire_bus(_handle, portMAX_DELAY);
    if (_cs != GPIO_NUM_NC) {
        gpio_set_level(_cs, 0);
    }
}

void AdapterSPI::ESPIDFImpl::endTransaction()
{
    if (!_in_transaction) {
        M5_LIB_LOGE("Don't nest!");
        return;
    }
    if (_cs != GPIO_NUM_NC) {
        gpio_set_level(_cs, 1);
    }
    spi_device_release_bus(_handle);
    _in_transaction = false;
}

m5::hal::error::error_t AdapterSPI::ESPIDFImpl::do_transmit(const uint8_t* tx, uint8_t* rx, const size_t len)
{
    const uint8_t* tp = tx;
    uint8_t* rp       = rx;
    size_t remain     = len;
    while (remain) {
        const size_t n = (remain > 64) ? 64 : remain;
        spi_transaction_t t{};
        t.length = n * 8;  // In bits
        if (n <= 4) {
            // Inline 4-byte path: no DMA buffer required (always safe)
            t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
            for (size_t i = 0; i < n; ++i) {
                t.tx_data[i] = tp ? tp[i] : 0xFF;  // Send 0xFF dummy on read (matches Arduino)
            }
        } else {
            // 4 < n <= 64: FIFO via DMA-disabled bus accepts stack buffers
            t.tx_buffer = tp;  // nullptr -> driver sends zeros
            t.rx_buffer = rp;
        }
        const esp_err_t err = spi_device_polling_transmit(_handle, &t);
        if (err != ESP_OK) {
            return to_spi_error(err);
        }
        if (n <= 4 && rp) {
            for (size_t i = 0; i < n; ++i) {
                rp[i] = t.rx_data[i];
            }
        }
        if (tp) {
            tp += n;
        }
        if (rp) {
            rp += n;
        }
        remain -= n;
    }
    return m5::hal::error::error_t::OK;
}

m5::hal::error::error_t AdapterSPI::ESPIDFImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    if (!data) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    return do_transmit(nullptr, data, len);
}

m5::hal::error::error_t AdapterSPI::ESPIDFImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                     const uint32_t /* unused */)
{
    return do_transmit(data, nullptr, len);
}

m5::hal::error::error_t AdapterSPI::ESPIDFImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                     const size_t len, const uint32_t /* unused */)
{
    auto ret = do_transmit(&reg, nullptr, 1);
    if (data && len && ret == m5::hal::error::error_t::OK) {
        ret = do_transmit(data, nullptr, len);
    }
    return ret;
}

m5::hal::error::error_t AdapterSPI::ESPIDFImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                     const size_t len, const uint32_t /* unused */)
{
    m5::types::big_uint16_t r(reg);
    auto ret = do_transmit(r.data(), nullptr, r.size());
    if (data && len && ret == m5::hal::error::error_t::OK) {
        ret = do_transmit(data, nullptr, len);
    }
    return ret;
}

AdapterSPI::AdapterSPI(spi_device_handle_t handle, const gpio_num_t cs)
    : Adapter(Adapter::Type::SPI, new AdapterSPI::ESPIDFImpl(handle, cs))
{
    assert(_impl);
}
#endif

}  // namespace unit
}  // namespace m5
