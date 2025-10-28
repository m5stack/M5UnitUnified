/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitComponent.cpp
  @brief Base class for Unit Component
*/
#include "M5UnitComponent.hpp"
#include <M5Utility.hpp>
#include <algorithm>
#include <array>

using namespace m5::unit::types;

namespace m5 {
namespace unit {

const char Component::name[] = "";
const types::uid_t Component::uid{};
const types::attr_t Component::attr{};

Component::Component(const uint8_t addr) : _adapter{new Adapter()}, _addr{addr}
{
}

size_t Component::childrenSize() const
{
    size_t sz{};
    auto it = childBegin();
    while (it != childEnd()) {
        ++sz;
        ++it;
    }
    return sz;
}

bool Component::existsChild(const uint8_t ch) const
{
    if (!_child) {
        return false;
    }
    return std::any_of(childBegin(), childEnd(), [&ch](const Component& c) { return ch == c.channel(); });
}

bool Component::canAccessI2C() const
{
    return attribute() & attribute::AccessI2C;
}

bool Component::canAccessGPIO() const
{
    return attribute() & attribute::AccessGPIO;
}

bool Component::canAccessUART() const
{
    return attribute() & attribute::AccessUART;
}

bool Component::canAccessSPI() const
{
    return attribute() & attribute::AccessSPI;
}

bool Component::add(Component& c, const int16_t ch)
{
    if (childrenSize() >= _component_cfg.max_children) {
        M5_LIB_LOGE("Can't connect any more");
        return false;
    }
    if (existsChild(ch)) {
        M5_LIB_LOGE("Already connected an other unit at channel:%u", ch);
        return false;
    }
    if (isRegistered()) {
        M5_LIB_LOGE(
            "As the parent unit is already registered with the UnitUnified, no additional children can be added");
        return false;
    }
    if (c.isRegistered()) {
        M5_LIB_LOGE("Children already registered with UnitUnified cannot be added");
        return false;
    }

    if (!add_child(&c)) {
        return false;
    }
    c._channel = ch;
    return true;
}

bool Component::add_child(Component* c)
{
    if (!c || c->_parent || c->_prev || c->_next) {
        M5_LIB_LOGE("Invalid child [%s] %p / %p / %p", c ? c->deviceName() : "null", c ? c->_parent : nullptr,
                    c ? c->_next : nullptr, c ? c->_prev : nullptr);

        return false;
    }
    // Add to tail
    if (!_child) {
        _child = c;
    } else {
        auto last = _child;
        while (last->_next) {
            last = last->_next;
        }
        last->_next = c;
        c->_prev    = last;
    }

    c->_parent = this;
    return true;
}

Component* Component::child(const uint8_t ch) const
{
    auto it = childBegin();
    while (it != childEnd()) {
        if (it->channel() == ch) {
            return const_cast<Component*>(&*it);
        }
        ++it;
    }
    return nullptr;
}

bool Component::assign(m5::hal::bus::Bus* bus)
{
    if (_addr) {
        _adapter = std::make_shared<AdapterI2C>(bus, _addr, _component_cfg.clock);
    }
    return static_cast<bool>(_adapter);
}

bool Component::assign(TwoWire& wire)
{
    if (canAccessI2C() && _addr) {
        _adapter = std::make_shared<AdapterI2C>(wire, _addr, _component_cfg.clock);
        return static_cast<bool>(_adapter);
    }
    return false;
}

bool Component::assign(const int8_t rx_pin, const int8_t tx_pin)
{
    if (canAccessGPIO()) {
        _adapter = std::make_shared<AdapterGPIO>(rx_pin, tx_pin);
        return static_cast<bool>(_adapter);
    }
    return false;
}

bool Component::assign(HardwareSerial& serial)
{
    if (canAccessUART()) {
        _adapter = std::make_shared<AdapterUART>(serial);
        return static_cast<bool>(_adapter);
    }
    return false;
}

bool Component::assign(SPIClass& spi, const SPISettings& settings)
{
    if (canAccessSPI()) {
        _adapter = std::make_shared<AdapterSPI>(spi, settings, address() /* CS */);
        return static_cast<bool>(_adapter);
    }
    return false;
}

bool Component::selectChannel(const uint8_t ch)
{
    bool ret{true};
    if (hasParent()) {
        ret = _parent->selectChannel(channel());
    }
    return ret && (select_channel(ch) == m5::hal::error::error_t::OK);
}

m5::hal::error::error_t Component::readWithTransaction(uint8_t* data, const size_t len)
{
    selectChannel(channel());
    auto r = adapter()->readWithTransaction(data, len);
    return r;
}

m5::hal::error::error_t Component::writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t exparam)
{
    selectChannel(channel());
    return adapter()->writeWithTransaction(data, len, exparam);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
m5::hal::error::error_t Component::writeWithTransaction(const Reg reg, const uint8_t* data, const size_t len,
                                                        const bool stop)
{
    selectChannel(channel());
    return adapter()->writeWithTransaction(reg, data, len, stop);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::readRegister(const Reg reg, uint8_t* rbuf, const size_t len, const uint32_t delayMillis,
                             const bool stop)
{
    if (!writeRegister(reg, nullptr, 0U, stop)) {
        M5_LIB_LOGE("Failed to write");
        return false;
    }

    m5::utility::delay(delayMillis);
    return (readWithTransaction(rbuf, len) == m5::hal::error::error_t::OK);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::readRegister8(const Reg reg, uint8_t& result, const uint32_t delayMillis, const bool stop)
{
    return readRegister(reg, &result, 1, delayMillis, stop);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::read_register16E(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop,
                                 const bool endian)
{
    uint8_t tmp[2]{};
    auto ret = readRegister(reg, tmp, 2, delayMillis, stop);
    if (ret) {
        result = (tmp[!endian] << 8) | tmp[endian];
    }
    return ret;
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::read_register32E(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop,
                                 const bool endian)
{
    uint8_t tmp[4]{};
    auto ret = readRegister(reg, tmp, 4, delayMillis, stop);
    if (ret) {
        result = (tmp[0 + 3 * endian] | (tmp[1 + endian] << 8) | (tmp[2 - endian] << 16)) | (tmp[3 - 3 * endian] << 24);
    }
    return ret;
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::writeRegister(const Reg reg, const uint8_t* buf, const size_t len, const bool stop)
{
    return (sizeof(Reg) == 2
                ? writeWithTransaction(reg, buf, len, stop)
                : writeWithTransaction((uint8_t)(reg & 0xFF), buf, len, stop)) == m5::hal::error::error_t::OK;
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::writeRegister8(const Reg reg, const uint8_t value, const bool stop)
{
    return writeRegister(reg, &value, 1, stop);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::write_register16E(const Reg reg, const uint16_t value, const bool stop, const bool endian)
{
    uint8_t tmp[2]{};
    tmp[endian]  = value & 0xFF;
    tmp[!endian] = (value >> 8) & 0xFF;
    return writeRegister(reg, tmp, 2, stop);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::write_register32E(const Reg reg, const uint32_t value, const bool stop, const bool endian)
{
    uint8_t tmp[4]{};
    tmp[0 + endian * 3] = value & 0xFF;
    tmp[1 + endian]     = (value >> 8) & 0xFF;
    tmp[2 - endian]     = (value >> 16) & 0xFF;
    tmp[3 - endian * 3] = (value >> 24) & 0xFF;
    return writeRegister(reg, tmp, 4, stop);
}

bool Component::generalCall(const uint8_t* data, const size_t len)
{
    return adapter()->generalCall(data, len) == m5::hal::error::error_t::OK;
}

bool Component::pinModeRX(const gpio::Mode m)
{
    return adapter()->pinModeRX(m) == m5::hal::error::error_t::OK;
}

bool Component::writeDigitalRX(const bool high)
{
    return adapter()->writeDigitalRX(high) == m5::hal::error::error_t::OK;
}

bool Component::readDigitalRX(bool& high)
{
    return adapter()->readDigitalRX(high) == m5::hal::error::error_t::OK;
}

bool Component::writeAnalogRX(const uint16_t v)
{
    return adapter()->writeAnalogRX(v) == m5::hal::error::error_t::OK;
}

bool Component::readAnalogRX(uint16_t& v)
{
    return adapter()->readAnalogRX(v) == m5::hal::error::error_t::OK;
}

bool Component::pulseInRX(uint32_t& duration, const int state, const uint32_t timeout_us)
{
    return adapter()->pulseInRX(duration, state, timeout_us) == m5::hal::error::error_t::OK;
}

bool Component::pinModeTX(const gpio::Mode m)
{
    return adapter()->pinModeTX(m) == m5::hal::error::error_t::OK;
}

bool Component::writeDigitalTX(const bool high)
{
    return adapter()->writeDigitalTX(high) == m5::hal::error::error_t::OK;
}

bool Component::readDigitalTX(bool& high)
{
    return adapter()->readDigitalTX(high) == m5::hal::error::error_t::OK;
}

bool Component::writeAnalogTX(const uint16_t v)
{
    return adapter()->writeAnalogTX(v) == m5::hal::error::error_t::OK;
}

bool Component::readAnalogTX(uint16_t& v)
{
    return adapter()->readAnalogTX(v) == m5::hal::error::error_t::OK;
}

bool Component::pulseInTX(uint32_t& duration, const int state, const uint32_t timeout_us)
{
    return adapter()->pulseInTX(duration, state, timeout_us) == m5::hal::error::error_t::OK;
}

bool Component::changeAddress(const uint8_t addr)
{
    if (canAccessI2C() && m5::utility::isValidI2CAddress(addr)) {
        auto ad = asAdapter<AdapterI2C>(Adapter::Type::I2C);
        if (ad) {
            M5_LIB_LOGI("Change to address %x", addr);
            _addr = addr;
            ad->setAddress(addr);
            return true;
        }
    }
    M5_LIB_LOGE("Failed to change, %u, %x", canAccessI2C(), addr);
    return false;
}

std::string Component::debugInfo() const
{
    std::string tmp{};
    switch (_adapter->type()) {
        case Adapter::Type::I2C:
            tmp = m5::utility::formatString("%p:%u ADDR:%02X", _adapter.get(), _adapter.use_count(),
                                            asAdapter<AdapterI2C>(Adapter::Type::I2C)->address());
            break;
        case Adapter::Type::GPIO:
            tmp = m5::utility::formatString("%p:%u RX:%d TX:%d", _adapter.get(), _adapter.use_count(),
                                            asAdapter<AdapterGPIO>(Adapter::Type::GPIO)->rx_pin(),
                                            asAdapter<AdapterGPIO>(Adapter::Type::GPIO)->tx_pin());
            break;
        default:
            tmp = m5::utility::formatString("%p:%u Type:%d", _adapter.get(), _adapter.use_count(), _adapter->type());
            break;
    }
    return m5::utility::formatString("[%s]:ID{0X%08x}:%s CH:%d parent:%u children:%zu/%u", deviceName(), identifier(),
                                     tmp.c_str(), channel(), hasParent(), childrenSize(), _component_cfg.max_children);
}

// Explicit template instantiation
template bool Component::readRegister<uint8_t>(const uint8_t, uint8_t*, const size_t, const uint32_t, const bool);
template bool Component::readRegister<uint16_t>(const uint16_t, uint8_t*, const size_t, const uint32_t, const bool);
template bool Component::readRegister8<uint8_t>(const uint8_t, uint8_t&, const uint32_t, const bool);
template bool Component::readRegister8<uint16_t>(const uint16_t, uint8_t&, const uint32_t, const bool);
template bool Component::read_register16E<uint8_t>(const uint8_t, uint16_t&, const uint32_t, const bool, const bool);
template bool Component::read_register16E<uint16_t>(const uint16_t, uint16_t&, const uint32_t, const bool, const bool);
template bool Component::read_register32E<uint8_t>(const uint8_t, uint32_t&, const uint32_t, const bool, const bool);
template bool Component::read_register32E<uint16_t>(const uint16_t, uint32_t&, const uint32_t, const bool, const bool);

template bool Component::writeRegister<uint8_t>(const uint8_t, const uint8_t*, const size_t, const bool);
template bool Component::writeRegister<uint16_t>(const uint16_t, const uint8_t*, const size_t, const bool);
template bool Component::writeRegister8<uint8_t>(const uint8_t, const uint8_t, const bool);
template bool Component::writeRegister8<uint16_t>(const uint16_t, const uint8_t, const bool);
template bool Component::write_register16E<uint8_t>(const uint8_t, const uint16_t, const bool, const bool);
template bool Component::write_register16E<uint16_t>(const uint16_t, const uint16_t, const bool, const bool);
template bool Component::write_register32E<uint8_t>(const uint8_t, const uint32_t, const bool, const bool);
template bool Component::write_register32E<uint16_t>(const uint16_t, const uint32_t, const bool, const bool);

template m5::hal::error::error_t Component::writeWithTransaction<uint8_t>(const uint8_t reg, const uint8_t* data,
                                                                          const size_t len, const bool stop);
template m5::hal::error::error_t Component::writeWithTransaction<uint16_t>(const uint16_t reg, const uint8_t* data,
                                                                           const size_t len, const bool stop);

}  // namespace unit
}  // namespace m5
