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

namespace m5 {
namespace unit {

const char Component::name[] = "";
const types::uid_t Component::uid{0};
const types::attr_t Component::attr{0};

Component::Component(const uint8_t addr) : _adapter{new Adapter(addr)}, _addr{addr}
{
    assert(_adapter);
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
        _adapter.reset(new Adapter(bus, _addr));
        _adapter->setClock(_component_cfg.clock);
    }
    return static_cast<bool>(_adapter);
}

bool Component::assign(TwoWire& wire)
{
    if (_addr) {
        _adapter.reset(new Adapter(wire, _addr));
        _adapter->setClock(_component_cfg.clock);
    }
    return static_cast<bool>(_adapter);
}

bool Component::selectChannel(const uint8_t ch)
{
    if (ch != 255) {
        M5_LIB_LOGV("%s:%u", deviceName(), ch);
    }
    bool ret{true};
    if (hasParent()) {
        ret = _parent->selectChannel(channel());
    }
    return ret && (select_channel(ch) == m5::hal::error::error_t::OK);
}

m5::hal::error::error_t Component::readWithTransaction(uint8_t* data, const size_t len)
{
    selectChannel(channel());
    auto r = _adapter->readWithTransaction(data, len);
    return r;
}

m5::hal::error::error_t Component::writeWithTransaction(const uint8_t* data, const size_t len, const bool stop)
{
    selectChannel(channel());
    return _adapter->writeWithTransaction(data, len, stop);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
m5::hal::error::error_t Component::writeWithTransaction(const Reg reg, const uint8_t* data, const size_t len,
                                                        const bool stop)
{
    selectChannel(channel());
    return _adapter->writeWithTransaction(reg, data, len, stop);
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
bool Component::readRegister16(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop)
{
    m5::types::big_uint16_t buf{};
    auto ret = readRegister(reg, buf.data(), buf.size(), delayMillis, stop);
    if (ret) {
        result = buf.get();
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
bool Component::writeRegister16(const Reg reg, const uint16_t value, const bool stop)
{
    m5::types::big_uint16_t u16{value};
    return writeRegister(reg, u16.data(), u16.size(), stop);
}

bool Component::generalCall(const uint8_t* data, const size_t len)
{
    return _adapter->generalCall(data, len) == m5::hal::error::error_t::OK;
}

bool Component::changeAddress(const uint8_t addr)
{
    if (m5::utility::isValidI2CAddress(addr)) {
        M5_LIB_LOGI("Change to address %x", addr);
        _addr = addr;
        _adapter.reset(_adapter->duplicate(addr));
        return true;
    }
    M5_LIB_LOGE("Invalid address %x", addr);
    return false;
}

std::string Component::debugInfo() const
{
    return m5::utility::formatString("[%s]:ID{0X%08x}:ADDR{0X%02x/0X%02x} parent:%u children:%zu", deviceName(),
                                     identifier(), address(), _adapter->address(), hasParent(), childrenSize());
}

// Explicit template instantiation
template bool Component::readRegister<uint8_t>(const uint8_t, uint8_t*, const size_t, const uint32_t, const bool);
template bool Component::readRegister<uint16_t>(const uint16_t, uint8_t*, const size_t, const uint32_t, const bool);
template bool Component::readRegister8<uint8_t>(const uint8_t, uint8_t&, const uint32_t, const bool);
template bool Component::readRegister8<uint16_t>(const uint16_t, uint8_t&, const uint32_t, const bool);
template bool Component::readRegister16<uint8_t>(const uint8_t, uint16_t&, const uint32_t, const bool);
template bool Component::readRegister16<uint16_t>(const uint16_t, uint16_t&, const uint32_t, const bool);

template bool Component::writeRegister<uint8_t>(const uint8_t, const uint8_t*, const size_t, const bool);
template bool Component::writeRegister<uint16_t>(const uint16_t, const uint8_t*, const size_t, const bool);
template bool Component::writeRegister8<uint8_t>(const uint8_t, const uint8_t, const bool);
template bool Component::writeRegister8<uint16_t>(const uint16_t, const uint8_t, const bool);
template bool Component::writeRegister16<uint8_t>(const uint8_t, const uint16_t, const bool);
template bool Component::writeRegister16<uint16_t>(const uint16_t, const uint16_t, const bool);

template m5::hal::error::error_t Component::writeWithTransaction<uint8_t>(const uint8_t reg, const uint8_t* data,
                                                                          const size_t len, const bool stop);
template m5::hal::error::error_t Component::writeWithTransaction<uint16_t>(const uint16_t reg, const uint8_t* data,
                                                                           const size_t len, const bool stop);

}  // namespace unit
}  // namespace m5
