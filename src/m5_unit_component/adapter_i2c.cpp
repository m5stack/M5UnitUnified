/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file adapter_i2c.cpp
  @brief Adapter for I2C to treat M5HAL, TwoWire, and I2C_Class in the same way
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
#if __has_include(<utility/I2C_Class.hpp>)
#include <utility/I2C_Class.hpp>
#define M5_UNITUNIFIED_ADAPTER_HAS_M5_I2C_CLASS
#endif

namespace m5 {
namespace unit {

#if defined(ARDUINO)

namespace {

// The input-signal selection field of gpio_func_in_sel_cfg_reg_t differs by chip generation
// (independent of the ESP-IDF version):
//   Older chips (ESP32/S2/S3/C3):     func_sel
//   Newer chips (C6/H2/P4 and later): in_sel
// Detect which member exists at compile time and read whichever is present (no chip defines needed).
template <class T>
auto read_func_in_sel(const volatile T& reg, int) -> decltype(+reg.in_sel)
{
    return reg.in_sel;
}
template <class T>
auto read_func_in_sel(const volatile T& reg, long) -> decltype(+reg.func_sel)
{
    return reg.func_sel;
}

int16_t search_pin_number(const int peripheral_sig)
{
    int16_t no{-1};
    no = (peripheral_sig >= 0 && peripheral_sig < m5::stl::size(GPIO.func_in_sel_cfg))
             ? static_cast<int16_t>(read_func_in_sel(GPIO.func_in_sel_cfg[peripheral_sig], 0))
             : -1;
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

#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
#pragma message "ESP-IDF I2C backend: i2c_master (new driver)"
namespace {
constexpr int default_i2c_timeout_ms = 1000;

m5::hal::error::error_t to_i2c_error(const esp_err_t err)
{
    switch (err) {
        case ESP_OK:
            return m5::hal::error::error_t::OK;
        case ESP_ERR_INVALID_ARG:
        case ESP_ERR_INVALID_STATE:
            return m5::hal::error::error_t::INVALID_ARGUMENT;
        default:
            return m5::hal::error::error_t::I2C_BUS_ERROR;
    }
}
}  // namespace

AdapterI2C::ESPIDFMasterBusImpl::ESPIDFMasterBusImpl(i2c_master_bus_handle_t bus, const uint8_t addr,
                                                     const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock), _bus(bus)
{
}

bool AdapterI2C::ESPIDFMasterBusImpl::begin()
{
    return ensure_device() == m5::hal::error::error_t::OK;
}

bool AdapterI2C::ESPIDFMasterBusImpl::end()
{
    _pending_write.clear();
    if (!_dev) {
        return true;
    }

    auto err = i2c_master_bus_rm_device(_dev);
    if (err == ESP_OK) {
        _dev = nullptr;
    }
    return err == ESP_OK;
}

AdapterI2C::I2CImpl* AdapterI2C::ESPIDFMasterBusImpl::duplicate(const uint8_t addr)
{
    return new ESPIDFMasterBusImpl(_bus, addr, _clock);
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::ensure_device()
{
    if (!_bus || !_addr) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    if (_dev) {
        return m5::hal::error::error_t::OK;
    }

    i2c_device_config_t dev_cfg{};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address  = _addr;
    dev_cfg.scl_speed_hz    = _clock;
    return to_i2c_error(i2c_master_bus_add_device(_bus, &dev_cfg, &_dev));
}

void AdapterI2C::ESPIDFMasterBusImpl::set_pending_write(const uint8_t* data, const size_t len)
{
    _pending_write.clear();
    if (data && len) {
        _pending_write.assign(data, data + len);
    }
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::transmit(const uint8_t* data, const size_t len)
{
    auto err = ensure_device();
    if (err != m5::hal::error::error_t::OK) {
        return err;
    }
    if (!data && len) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    if (!len) {
        return m5::hal::error::error_t::OK;
    }
    return to_i2c_error(i2c_master_transmit(_dev, data, len, default_i2c_timeout_ms));
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    auto err = ensure_device();
    if (err != m5::hal::error::error_t::OK) {
        _pending_write.clear();
        return err;
    }
    if (!data && len) {
        _pending_write.clear();
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    if (!len) {
        _pending_write.clear();
        return m5::hal::error::error_t::OK;
    }

    esp_err_t ret{};
    if (_pending_write.empty()) {
        ret = i2c_master_receive(_dev, data, len, default_i2c_timeout_ms);
    } else {
        ret = i2c_master_transmit_receive(_dev, _pending_write.data(), _pending_write.size(), data, len,
                                          default_i2c_timeout_ms);
        _pending_write.clear();
    }
    return to_i2c_error(ret);
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                              const uint32_t stop)
{
    if (!stop) {
        set_pending_write(data, len);
        return m5::hal::error::error_t::OK;
    }
    _pending_write.clear();
    return transmit(data, len);
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                              const size_t len, const uint32_t stop)
{
    std::vector<uint8_t> tx;
    tx.reserve(1 + len);
    tx.push_back(reg);
    if (data && len) {
        tx.insert(tx.end(), data, data + len);
    }
    if (!stop) {
        _pending_write.swap(tx);
        return m5::hal::error::error_t::OK;
    }
    _pending_write.clear();
    return transmit(tx.data(), tx.size());
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                              const size_t len, const uint32_t stop)
{
    m5::types::big_uint16_t r(reg);
    std::vector<uint8_t> tx;
    tx.reserve(r.size() + len);
    tx.insert(tx.end(), r.data(), r.data() + r.size());
    if (data && len) {
        tx.insert(tx.end(), data, data + len);
    }
    if (!stop) {
        _pending_write.swap(tx);
        return m5::hal::error::error_t::OK;
    }
    _pending_write.clear();
    return transmit(tx.data(), tx.size());
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::generalCall(const uint8_t* data, const size_t len)
{
    if (!_bus || (!data && len)) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    if (!len) {
        return m5::hal::error::error_t::OK;
    }

    i2c_master_dev_handle_t dev{};
    i2c_device_config_t dev_cfg{};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address  = 0x00;
    dev_cfg.scl_speed_hz    = _clock;
    auto err                = i2c_master_bus_add_device(_bus, &dev_cfg, &dev);
    if (err == ESP_OK) {
        err = i2c_master_transmit(dev, data, len, default_i2c_timeout_ms);
        i2c_master_bus_rm_device(dev);
    }
    return to_i2c_error(err);
}

m5::hal::error::error_t AdapterI2C::ESPIDFMasterBusImpl::wakeup()
{
    if (!_bus || !_addr) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    return to_i2c_error(i2c_master_probe(_bus, _addr, default_i2c_timeout_ms));
}
#elif defined(ESP_PLATFORM)
#pragma message "ESP-IDF I2C backend: legacy driver/i2c.h"

AdapterI2C::ESPIDFLegacyBusImpl::ESPIDFLegacyBusImpl(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl,
                                                     const uint8_t addr, const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock), _port(port), _sda(static_cast<int16_t>(sda)), _scl(static_cast<int16_t>(scl))
{
    // The legacy driver has no per-device clock (unlike the IDF >= 5.2 master driver, which applies
    // each device's scl_speed_hz per transfer). The whole port shares one timing register set, so
    // every consumer must re-assert its own clock before each transfer. Compute this unit's period
    // once here (the driver is already installed by the wiring helper); transactions then re-apply it
    // via i2c_set_period. Each unit caches its own _high/_low, so units with different clocks on the
    // same port do not clobber each other.
    apply_clock();
}

void AdapterI2C::ESPIDFLegacyBusImpl::apply_clock()
{
    i2c_config_t conf{};
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = _sda;
    conf.scl_io_num       = _scl;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = _clock;
    // i2c_param_config applies timing to the HAL even without the driver installed, but
    // i2c_get_period returns ESP_FAIL (without writing) when the driver object is absent.
    // Mark the cached period invalid (0) when it cannot be read back; transactions then skip
    // i2c_set_period and rely on the timing param_config just applied.
    _high = 0;
    _low  = 0;
    if (i2c_param_config(_port, &conf) == ESP_OK) {
        if (i2c_get_period(_port, &_high, &_low) != ESP_OK) {
            _high = 0;
            _low  = 0;
        }
    }
    M5_LIB_LOGI("apply_clock port=%d clock=%u high=%d low=%d%s", (int)_port, (unsigned)_clock, _high, _low,
                (_high > 0 && _low > 0) ? "" : " (period unreadable; using param_config timing)");
}

bool AdapterI2C::ESPIDFLegacyBusImpl::begin()
{
    apply_clock();
    return true;
}

bool AdapterI2C::ESPIDFLegacyBusImpl::end()
{
    // Borrowed driver: do not uninstall (owned by the user)
    return true;
}

AdapterI2C::I2CImpl* AdapterI2C::ESPIDFLegacyBusImpl::duplicate(const uint8_t addr)
{
    auto* p =
        new ESPIDFLegacyBusImpl(_port, static_cast<gpio_num_t>(_sda), static_cast<gpio_num_t>(_scl), addr, _clock);
    p->_high = _high;
    p->_low  = _low;
    return p;
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    if (!data || !len) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    if (_high > 0 && _low > 0) {
        i2c_set_period(_port, _high, _low);
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, static_cast<uint8_t>((_addr << 1) | I2C_MASTER_READ), true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (err == ESP_OK) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::write_with_transaction(const uint8_t addr, const uint8_t* data,
                                                                                const size_t len, const uint32_t stop)
{
    if (_high > 0 && _low > 0) {
        i2c_set_period(_port, _high, _low);
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, static_cast<uint8_t>((addr << 1) | I2C_MASTER_WRITE), true);
    if (data && len) {
        i2c_master_write(cmd, data, len, true);
    }
    if (stop) {
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (err == ESP_OK) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                              const uint32_t stop)
{
    return write_with_transaction(_addr, data, len, stop);
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                              const size_t len, const uint32_t stop)
{
    if (_high > 0 && _low > 0) {
        i2c_set_period(_port, _high, _low);
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, static_cast<uint8_t>((_addr << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(cmd, reg, true);
    if (data && len) {
        i2c_master_write(cmd, data, len, true);
    }
    if (stop) {
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (err == ESP_OK) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                              const size_t len, const uint32_t stop)
{
    m5::types::big_uint16_t r(reg);
    if (_high > 0 && _low > 0) {
        i2c_set_period(_port, _high, _low);
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, static_cast<uint8_t>((_addr << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write(cmd, r.data(), r.size(), true);
    if (data && len) {
        i2c_master_write(cmd, data, len, true);
    }
    if (stop) {
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return (err == ESP_OK) ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::generalCall(const uint8_t* data, const size_t len)
{
    return write_with_transaction(0x00, data, len, true);
}

m5::hal::error::error_t AdapterI2C::ESPIDFLegacyBusImpl::wakeup()
{
    return write_with_transaction(_addr, nullptr, 0, true);
}

#endif

// Impl for I2C_Class
#if defined(M5_UNITUNIFIED_ADAPTER_HAS_M5_I2C_CLASS)

AdapterI2C::I2CClassImpl::I2CClassImpl(m5::I2C_Class& i2c, const uint8_t addr, const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock), _i2c(&i2c)
{
    _sda = _i2c->getSDA();
    _scl = _i2c->getSCL();
    M5_LIB_LOGI("I2C_Class SDA:%d, SCL:%d", _sda, _scl);
}

bool AdapterI2C::I2CClassImpl::begin()
{
    return _i2c->begin();
}

bool AdapterI2C::I2CClassImpl::end()
{
    return _i2c->release();
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::readWithTransaction(uint8_t* data, const size_t len)
{
    assert(_addr);
    if (!data) {
        return m5::hal::error::error_t::INVALID_ARGUMENT;
    }
    bool started = _in_transaction ? _i2c->restart(_addr, true, _clock) : _i2c->start(_addr, true, _clock);
    if (!started) {
        _in_transaction = false;
        return m5::hal::error::error_t::I2C_BUS_ERROR;
    }
    bool ok = _i2c->read(data, len, true);
    _i2c->stop();
    _in_transaction = false;
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint8_t* data, const size_t len,
                                                                       const uint32_t stop)
{
    assert(_addr);
    bool started = _in_transaction ? _i2c->restart(_addr, false, _clock) : _i2c->start(_addr, false, _clock);
    if (!started) {
        _in_transaction = false;
        return m5::hal::error::error_t::I2C_BUS_ERROR;
    }
    bool ok = true;
    if (data && len) {
        ok = _i2c->write(data, len);
    }
    if (stop || !ok) {
        _i2c->stop();
        _in_transaction = false;
    } else {
        _in_transaction = true;
    }
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint8_t reg, const uint8_t* data,
                                                                       const size_t len, const uint32_t stop)
{
    assert(_addr);
    bool started = _in_transaction ? _i2c->restart(_addr, false, _clock) : _i2c->start(_addr, false, _clock);
    if (!started) {
        _in_transaction = false;
        return m5::hal::error::error_t::I2C_BUS_ERROR;
    }
    bool ok = _i2c->write(reg);
    if (ok && data && len) {
        ok = _i2c->write(data, len);
    }
    if (stop || !ok) {
        _i2c->stop();
        _in_transaction = false;
    } else {
        _in_transaction = true;
    }
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint16_t reg, const uint8_t* data,
                                                                       const size_t len, const uint32_t stop)
{
    assert(_addr);
    m5::types::big_uint16_t r(reg);
    bool started = _in_transaction ? _i2c->restart(_addr, false, _clock) : _i2c->start(_addr, false, _clock);
    if (!started) {
        _in_transaction = false;
        return m5::hal::error::error_t::I2C_BUS_ERROR;
    }
    bool ok = _i2c->write(r.data(), r.size());
    if (ok && data && len) {
        ok = _i2c->write(data, len);
    }
    if (stop || !ok) {
        _i2c->stop();
        _in_transaction = false;
    } else {
        _in_transaction = true;
    }
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

AdapterI2C::I2CImpl* AdapterI2C::I2CClassImpl::duplicate(const uint8_t addr)
{
    return new I2CClassImpl(*_i2c, addr, _clock);
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::generalCall(const uint8_t* data, const size_t len)
{
    bool ok = _i2c->start(0x00, false, _clock);
    if (ok && data && len) {
        ok = _i2c->write(data, len);
    }
    _i2c->stop();
    _in_transaction = false;
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::wakeup()
{
    bool ok = _i2c->start(_addr, false, _clock);
    _i2c->stop();
    _in_transaction = false;
    return ok ? m5::hal::error::error_t::OK : m5::hal::error::error_t::I2C_BUS_ERROR;
}

#else
// Stub when I2C_Class is not available
AdapterI2C::I2CClassImpl::I2CClassImpl(m5::I2C_Class& i2c, const uint8_t addr, const uint32_t clock)
    : AdapterI2C::I2CImpl(addr, clock)
{
    (void)i2c;
    M5_LIB_LOGE("I2C_Class not available");
}

bool AdapterI2C::I2CClassImpl::begin()
{
    return false;
}

bool AdapterI2C::I2CClassImpl::end()
{
    return false;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::readWithTransaction(uint8_t*, const size_t)
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint8_t*, const size_t, const uint32_t)
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint8_t, const uint8_t*, const size_t,
                                                                       const uint32_t)
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::writeWithTransaction(const uint16_t, const uint8_t*, const size_t,
                                                                       const uint32_t)
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

AdapterI2C::I2CImpl* AdapterI2C::I2CClassImpl::duplicate(const uint8_t addr)
{
    return new I2CImpl(addr, _clock);
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::generalCall(const uint8_t*, const size_t)
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

m5::hal::error::error_t AdapterI2C::I2CClassImpl::wakeup()
{
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

#endif

// Adapter
#if defined(ARDUINO)
AdapterI2C::AdapterI2C(TwoWire& wire, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::WireImpl(wire, addr, clock))
{
    assert(_impl);
}
#endif

AdapterI2C::AdapterI2C(m5::hal::bus::Bus* bus, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::BusImpl(bus, addr, clock))
{
    assert(_impl);
}

#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
AdapterI2C::AdapterI2C(i2c_master_bus_handle_t bus, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::ESPIDFMasterBusImpl(bus, addr, clock))
{
    assert(_impl);
}
#elif defined(ESP_PLATFORM)
AdapterI2C::AdapterI2C(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl, const uint8_t addr,
                       const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::ESPIDFLegacyBusImpl(port, sda, scl, addr, clock))
{
    assert(_impl);
}
#endif

#if defined(M5_UNITUNIFIED_ADAPTER_HAS_M5_I2C_CLASS)
#pragma message "Support I2C_Class"
AdapterI2C::AdapterI2C(m5::I2C_Class& i2c, const uint8_t addr, const uint32_t clock)
    : Adapter(Adapter::Type::I2C, new AdapterI2C::I2CClassImpl(i2c, addr, clock))
{
    assert(_impl);
}
#else
#pragma message "Not support I2C_Class"
AdapterI2C::AdapterI2C(m5::I2C_Class& i2c, const uint8_t addr, const uint32_t clock) : Adapter()
{
    (void)i2c;
    (void)addr;
    (void)clock;
    assert(_impl);
    M5_LIB_LOGE("Not support I2C_Class");
}
#endif

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
