/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_i2c.cpp
  @brief Adapter for I2C to treat M5HAL and TwoWire in the same way
  @note  Currently handles TwoWire directly, but will handle via M5HAL in the future
*/
#include "adapter_i2c.hpp"
#if defined(ARDUINO)
#include <Wire.h>
#endif
#include <M5HAL.hpp>
#include <M5Utility.hpp>
#include <soc/gpio_struct.h>
#include <soc/gpio_sig_map.h>
#include <cassert>

#if defined(ARDUINO)

namespace {
int16_t search_pin_number(const int peripheral_sig)
{
    int16_t no{-1};
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    // C6
    no = (peripheral_sig >= 0 && peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg))
             ? GPIO.func_in_sel_cfg[peripheral_sig].in_sel
             : -1;
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
    // P4
    no = (peripheral_sig >= 0 && peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg))
             ? GPIO.func_in_sel_cfg[peripheral_sig].in_sel
             : -1;
#else
    // Others
    no = (peripheral_sig >= 0 && peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg))
             ? GPIO.func_in_sel_cfg[peripheral_sig].func_sel
             : -1;
#endif

    return (no < GPIO_NUM_MAX) ? no : -1;
}

int8_t idx_table[][2] = {
#if !defined(CONFIG_IDF_TARGET_ESP32P4)
    {I2CEXT0_SDA_IN_IDX, I2CEXT0_SCL_IN_IDX},  // Wire
#if !defined(CONFIG_IDF_TARGET_ESP32C6)
    {I2CEXT1_SDA_IN_IDX, I2CEXT1_SCL_IN_IDX},  // Wire1
#else
    {I2CEXT0_SDA_IN_IDX, I2CEXT0_SCL_IN_IDX},  // Same as Wire
#endif
#else
    {I2C0_SDA_PAD_IN_IDX, I2C0_SCL_PAD_IN_IDX},  // Wire
    {I2C1_SDA_PAD_IN_IDX, I2C1_SCL_PAD_IN_IDX},  // Wire1
#endif
};
}  // namespace

namespace m5 {
namespace unit {

// Impl for TwoWire
AdapterI2C::WireImpl::WireImpl(TwoWire& wire, const uint8_t addr, const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock), _wire(&wire)
{
    uint32_t w = (&wire != &Wire);
    _sda       = search_pin_number(idx_table[w][0]);
    _scl       = search_pin_number(idx_table[w][1]);
    M5_LIB_LOGI("I2C SDA:%d, SCL:%d", _sda, _scl);
}

bool AdapterI2C::WireImpl::begin()
{
    return _wire->begin();
}
bool AdapterI2C::WireImpl::end()
{
#if defined(WIRE_HAS_END)
    return _wire->end();
#else
    return false;
#endif
}

m5::hal::error::error_t AdapterI2C::WireImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    assert(_addr);
    if (data && _wire->requestFrom(_addr, len)) {
        auto count = std::min(len, (size_t)_wire->available());
        for (size_t i = 0; i < count; ++i) {
            data[i] = (uint8_t)_wire->read();
        }
        return (count == len) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
    }
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t AdapterI2C::WireImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                   const uint32_t stop)
{
    return write_with_transaction(_addr, data, len, stop);
}

m5::hal::error::error_t AdapterI2C::WireImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                   const size_t len, const uint32_t stop)
{
    assert(_addr);
    _wire->setClock(_clock);

    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if (data && len) {
        _wire->write(data, len);
    }
    auto ret = _wire->endTransmission(stop);
    if (ret) {
        M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
    }
    return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::WireImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                   const size_t len, const uint32_t stop)
{
    assert(_addr);
    _wire->setClock(_clock);

    m5::types::big_uint16_t r(reg);
    _wire->beginTransmission(_addr);
    _wire->write(r.data(), r.size());
    if (data && len) {
        _wire->write(data, len);
    }
    auto ret = _wire->endTransmission(stop);
    if (ret) {
        M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
    }
    return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

AdapterI2C::I2CImpl* AdapterI2C::WireImpl::duplicate(const uint8_t addr)
{
    return new WireImpl(*_wire, addr, _clock);
}

m5::hal::error::error_t AdapterI2C::WireImpl::generalCall(const uint8_t* data, const size_t len)
{
    return write_with_transaction(0x00, data, len, true);
}

m5::hal::error::error_t AdapterI2C::WireImpl::wakeup()
{
    return write_with_transaction(_addr, nullptr, 0, true);
}

m5::hal::error::error_t AdapterI2C::WireImpl::write_with_transaction(const uint8_t addr, const uint8_t* data,
                                                                     const size_t len, const uint32_t stop)
{
    _wire->setClock(_clock);
    _wire->beginTransmission(addr);
    if (data) {
        _wire->write(data, len);
    }
    auto ret = _wire->endTransmission(stop);
    if (ret) {
        M5_LIB_LOGE("%d endTransmission stop:%d", ret, stop);
    }
    return (ret == 0) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

#endif

// Impl for M5HAL
AdapterI2C::BusImpl::BusImpl(m5::hal::bus::Bus* bus, const uint8_t addr, const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock), _bus(bus)
{
    _access_cfg.i2c_addr = addr;
    _access_cfg.freq     = clock;
    if (_bus && _bus->getBusType() == m5::hal::types::bus_type_t::I2C) {
        auto& cfg = static_cast<const m5::hal::bus::I2CBusConfig&>(_bus->getConfig());
        _sda      = cfg.pin_sda ? cfg.pin_sda->getGpioNumber() : -1;
        _scl      = cfg.pin_scl ? cfg.pin_scl->getGpioNumber() : -1;
    }
    M5_LIB_LOGI("I2C SDA:%d, SCL:%d", _sda, _scl);
}

AdapterI2C::I2CImpl* AdapterI2C::BusImpl::duplicate(const uint8_t addr)
{
    return new BusImpl(_bus, addr, _clock);
}

m5::hal::error::error_t AdapterI2C::BusImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    if (_bus && data) {
        auto acc = _bus->beginAccess(_access_cfg);
        if (acc) {
            auto trans  = acc.value();
            auto result = trans->startRead().and_then([&trans, &data, &len]() {
                return trans->readLastNack(data, len).and_then([&trans](size_t&&) { return trans->stop(); });
            });
            // Clean-up must be called
            auto eresult = this->_bus->endAccess(std::move(trans));
            return result.error_or(eresult);
        }
        return acc.error();
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

m5::hal::error::error_t AdapterI2C::BusImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                  const uint32_t stop)
{
    if (_bus) {
        auto acc = _bus->beginAccess(_access_cfg);
        if (acc) {
            auto trans  = acc.value();
            auto result = trans->startWrite().and_then([&trans, &data, &len, &stop]() {
                return trans->write(data, len).and_then([&trans, &stop](size_t&&) {
                    return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                });
            });
            // Clean-up must be called
            auto eresult = this->_bus->endAccess(std::move(trans));
            return result.error_or(eresult);
        }
        return acc.error();
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

m5::hal::error::error_t AdapterI2C::BusImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                  const size_t len, const uint32_t stop)
{
    assert(_addr);

    if (_bus) {
        auto acc = _bus->beginAccess(_access_cfg);
        if (acc) {
            auto trans  = acc.value();
            auto result = trans->startWrite().and_then([&trans, &reg, &data, &len, &stop]() {
                return trans->write(&reg, 1).and_then([&trans, &data, &len, &stop](size_t&&) {
                    return ((data && len) ? trans->write(data, len)
                                          : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL))
                        .and_then([&trans, &stop](size_t&&) {
                            return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                        });
                });
            });
            // Clean-up must be called
            auto eresult = this->_bus->endAccess(std::move(trans));
            return result.error_or(eresult);
        }
        return acc.error();
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

m5::hal::error::error_t AdapterI2C::BusImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                  const size_t len, const uint32_t stop)
{
    assert(_addr);

    if (_bus) {
        auto acc = _bus->beginAccess(_access_cfg);
        if (acc) {
            m5::types::big_uint16_t r(reg);
            auto trans  = acc.value();
            auto result = trans->startWrite().and_then([&trans, &r, &data, &len, &stop]() {
                return trans->write(r.data(), r.size()).and_then([&trans, &data, &len, &stop](size_t&&) {
                    return ((data && len) ? trans->write(data, len)
                                          : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL))
                        .and_then([&trans, &stop](size_t&&) {
                            return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                        });
                });
            });

            // Clean-up must be called
            auto eresult = this->_bus->endAccess(std::move(trans));
            return result.error_or(eresult);
        }
        return acc.error();
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

m5::hal::error::error_t AdapterI2C::BusImpl::generalCall(const uint8_t* data, const size_t len)
{
    m5::hal::bus::I2CMasterAccessConfig gcfg = _access_cfg;
    gcfg.i2c_addr                            = 0x00;
    return write_with_transaction(gcfg, data, len, true);
}

m5::hal::error::error_t AdapterI2C::BusImpl::wakeup()
{
    return write_with_transaction(_access_cfg, nullptr, 0, true);
}

m5::hal::error::error_t AdapterI2C::BusImpl::write_with_transaction(const m5::hal::bus::I2CMasterAccessConfig& cfg,
                                                                    const uint8_t* data, const size_t len,
                                                                    const uint32_t stop)
{
    if (_bus) {
        auto acc = _bus->beginAccess(cfg);
        if (acc) {
            auto trans = acc.value();
            auto result =
                trans->startWrite()
                    .and_then([&trans, &data, &len]() {
                        return ((data && len) ? trans->write(data, len)
                                              : m5::stl::expected<size_t, m5::hal::error::error_t>((size_t)0UL));
                    })
                    .and_then([&trans, &stop](size_t&&) {
                        return stop ? trans->stop() : m5::stl::expected<void, m5::hal::error::error_t>();
                    });
            // Clean-up must be called
            auto eresult = this->_bus->endAccess(std::move(trans));
            return result.error_or(eresult);
        }
        return acc.error();
    }
    return m5::hal::error::error_t::INVALID_ARGUMENT;
}

// Adapter
#if defined(ARDUINO)
AdapterI2C::AdapterI2C(TwoWire& wire, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::WireImpl(wire, addr, clock))
{
    assert(_impl);
}

#else
#pragma message "Not support TwoWire"
AdapterI2C::AdapterI2C(TwoWire& wire, const uint8_t addr, const uint32_t clock) : Adapter()
{
    (void)wire;
    (void)addr;
    (void)clock;
    assert(_impl);
    M5_LIB_LOGE("Not support TwoWire");
}
#endif

AdapterI2C::AdapterI2C(m5::hal::bus::Bus* bus, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::BusImpl(bus, addr, clock))
{
    assert(_impl);
}

Adapter* AdapterI2C::duplicate(const uint8_t addr)
{
    auto ptr = new AdapterI2C();
    if (ptr) {
        ptr->_impl.reset(impl()->duplicate(addr));
        if (ptr->_impl) {
            return ptr;
        }
        delete ptr;
    }
    M5_LIB_LOGE("Failed to duplicate");
    return nullptr;
}

bool AdapterI2C::pushPin()
{
#if defined(ARDUINO)
    if (_backupSCL.getPin() < 0 && _backupSDA.getPin() < 0) {
        _backupSCL.setPin(scl());
        _backupSCL.backup();
        _backupSDA.setPin(sda());
        _backupSDA.backup();
        M5_LIB_LOGD(">>Push SCL:%u SDA:%u", _backupSCL.getPin(), _backupSDA.getPin());
        return true;
    }
    return false;

#else
    return false;
#endif
}

bool AdapterI2C::popPin()
{
#if defined(ARDUINO)
    if (_backupSCL.getPin() >= 0 && _backupSDA.getPin() >= 0) {
        M5_LIB_LOGD("<<Pop SCL:%u SDA:%u", _backupSCL.getPin(), _backupSDA.getPin());

        _backupSCL.restore();
        _backupSDA.restore();
        _backupSCL.setPin(-1);
        _backupSDA.setPin(-1);
        return true;
    }
    return false;
#else
    return false;
#endif
}

}  // namespace unit
}  // namespace m5
