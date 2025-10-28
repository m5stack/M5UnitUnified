/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_spi.cpp
  @brief Adapters to treat M5HAL and SPI in the same way using Searial
  @note  Currently handles SPI directly, but will handle via M5HAL in the future
*/
#include "adapter_spi.hpp"
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <cassert>

namespace m5 {
namespace unit {

#if defined(ARDUINO)
uint32_t AdapterSPI::SPIClassImpl::transaction_count{};

AdapterSPI::SPIClassImpl::SPIClassImpl(SPIClass& spi, const SPISettings& settings, const uint8_t cs)
    : AdapterSPI::SPIImpl(cs), _spi(&spi), _settings{settings}
{
}

void AdapterSPI::SPIClassImpl::beginTransaction()
{
    if (transaction_count++ == 0) {
        //                        M5_LIB_LOGE(">>>> SPI Transaction");
        _spi->beginTransaction(_settings);
        digitalWrite(cs_pin(), LOW);
    }
}

void AdapterSPI::SPIClassImpl::endTransaction()
{
    if (transaction_count && --transaction_count == 0) {
        //                   M5_LIB_LOGE("<<<< SPI Transaction");
        digitalWrite(cs_pin(), HIGH);
        _spi->endTransaction();
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
                                                                       const uint32_t /* unuse */)
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

AdapterSPI::AdapterSPI(SPIClass& spi, const SPISettings& settings, const uint8_t cs)
    : Adapter(Adapter::Type::SPI, new AdapterSPI::SPIClassImpl(spi, settings, cs))
{
    assert(_impl);
}
#endif

}  // namespace unit
}  // namespace m5
