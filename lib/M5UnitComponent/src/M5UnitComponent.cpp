/*!
  @file M5UnitComponent.cpp
  @brief Base class for Unit Component


  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "M5UnitComponent.hpp"
#include <M5Utility.hpp>
#include <algorithm>
#include <array>
// For alloca() see also (*1)
#if __has_include(<alloca.h>)
#include <alloca.h>
#else
#include <stdlib.h>
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#endif

namespace m5 {
namespace unit {

const char Component::name[] = "";
const types::uid_t Component::uid{0};
const types::attr_t Component::attr{0};

Component::Component(const uint8_t addr)
    : _adapter{new Adapter(addr)}, _addr{addr} {
}

Component::Component(Component&& o)
    : _manager{o._manager},
      _adapter{std::move(o._adapter)},
      _order{o._order},
      _uccfg{o._uccfg},
      _channel{o._channel},
      _addr{o._addr},
      _parent{o._parent},
      _next{o._next},
      _prev{o._prev},
      _child{o._child} {
    o._manager = nullptr;
    o._order   = 0;
    o._uccfg   = {};
    o._channel = -1;
    o._addr    = 0x00;
    o._parent = o._next = o._prev = o._child = nullptr;
}

Component& Component::operator=(Component&& o) {
    if (this != &o) {
        _manager = o._manager;
        _adapter = std::move(o._adapter);
        _order   = o._order;
        _uccfg   = o._uccfg;
        _channel = o._channel;
        _addr    = o._addr;
        _parent  = o._parent;
        _next    = o._next;
        _prev    = o._prev;
        _child   = o._child;

        o._manager = nullptr;
        o._adapter = nullptr;
        o._order   = 0;
        o._uccfg   = {};
        o._channel = -1;
        o._addr    = 0x00;
        o._parent = o._next = o._prev = o._child = nullptr;
    }
    return *this;
}

size_t Component::childrenSize() const {
    size_t sz{};
    auto it = childBegin();
    while (it != childEnd()) {
        ++sz;
        ++it;
    }
    return sz;
}

bool Component::exists(const uint8_t ch) const {
    if (!_child) {
        return false;
    }
    return std::any_of(childBegin(), childEnd(), [&ch](const Component& c) {
        return c.channel() != 0xFF && ch == c.channel();
    });
}

bool Component::add(Component& c, const int16_t ch) {
    if (childrenSize() >= _uccfg.max_children) {
        M5_LIB_LOGE("Can't connect any more");
        return false;
    }
    if (exists(ch)) {
        M5_LIB_LOGE("Already connected channel:%u", ch);
        return false;
    }
    if (isRegistered()) {
        M5_LIB_LOGE(
            "As the parent unit is already registered with the UnitUnified, no "
            "additional children can be added.");
        return false;
    }
    if (c.isRegistered()) {
        M5_LIB_LOGE(
            "Children already registered with UnitUnified cannot be added.");
        return false;
    }

    if (!add_child(&c)) {
        return false;
    }
    c._channel = ch;
    return true;
}

bool Component::add_child(Component* c) {
    if (!c || c->_parent || c->_prev || c->_next) {
        M5_LIB_LOGE("Invalid child [%s] %p / %p / %p",
                    c ? c->deviceName() : "null", c ? c->_parent : nullptr,
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

Component* Component::child(const uint8_t ch) const {
    auto it = childBegin();
    while (it != childEnd()) {
        if (it->channel() == ch) {
            return const_cast<Component*>(&*it);
        }
        ++it;
    }
    return nullptr;
}

bool Component::assign(m5::hal::bus::Bus* bus) {
    if (_addr) {
        _adapter.reset(new Adapter(bus, _addr));
    }
    return static_cast<bool>(_adapter);
}

bool Component::assign(TwoWire& wire) {
    if (_addr) {
        _adapter.reset(new Adapter(wire, _addr));
    }
    return static_cast<bool>(_adapter);
}

bool Component::selectChannel(const uint8_t ch) {
    M5_LIB_LOGV("%s:%u", deviceName(), ch);
    bool ret{true};
    if (hasParent()) {
        ret = _parent->selectChannel(channel());
    }
    return ret && (select_channel(ch) == m5::hal::error::error_t::OK);
}

m5::hal::error::error_t Component::readWithTransaction(uint8_t* data,
                                                       const size_t len) {
    selectChannel(channel());
    auto r = _adapter->readWithTransaction(data, len);
    return r;
}

m5::hal::error::error_t Component::writeWithTransaction(const uint8_t* data,
                                                        const size_t len) {
    selectChannel(channel());
    auto r = _adapter->writeWithTransaction(data, len);
    return r;
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::readRegister(const Reg reg, uint8_t* rbuf, const size_t len,
                             const uint32_t delayMillis) {
    if (!writeRegister(reg)) {
        M5_LIB_LOGE("Failed to write");
        return false;
    }

    m5::utility::delay(delayMillis);

    return (readWithTransaction(rbuf, len) == m5::hal::error::error_t::OK);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::readRegister8(const Reg reg, uint8_t& result,
                              const uint32_t delayMillis) {
    return readRegister(reg, &result, 1, delayMillis);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::readRegister16(const Reg reg, uint16_t& result,
                               const uint32_t delayMillis) {
    m5::types::big_uint16_t buf{};
    auto ret = readRegister(reg, buf.data(), buf.size(), delayMillis);
    if (ret) {
        result = buf.get();
    }
    return ret;
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::writeRegister(const Reg reg, const uint8_t* buf,
                              const size_t len) {
#if 1
    static_assert(sizeof(reg) <= 2, "overflow");
    static_assert(std::is_integral<Reg>::value && std::is_unsigned<Reg>::value,
                  "Type must be unsigned integer");

    // TODO : Change to a form that does not use alloca() (*1)

    const size_t bsize = sizeof(reg) + len;
    uint8_t* wbuf      = static_cast<uint8_t*>(alloca(bsize));
    // Placement new
    new (wbuf)
        m5::types::big_uint16_t(sizeof(Reg) == 2 ? reg : ((uint16_t)reg) << 8U);
    // Overwrite wbuf[1] if Reg is uint8_t
    memcpy(wbuf + sizeof(reg), buf, len);

    return (writeWithTransaction(wbuf, bsize) == m5::hal::error::error_t::OK);
#else
    m5::types::big_uint16_t r(sizeof(Reg) == 2 ? reg : ((uint16_t)reg) << 8U);
    return writeWithTransaction(r.data(), sizeof(Reg)) ==
               m5::hal::error::error_t::OK &&
           writeWithTransaction(buf, len) == m5::hal::error::error_t::OK;

#endif
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::writeRegister8(const Reg reg, const uint8_t value) {
    return writeRegister(reg, &value, 1);
}

template <typename Reg,
          typename std::enable_if<std::is_integral<Reg>::value &&
                                      std::is_unsigned<Reg>::value &&
                                      sizeof(Reg) <= 2,
                                  std::nullptr_t>::type>
bool Component::writeRegister16(const Reg reg, const uint16_t value) {
    m5::types::big_uint16_t u16{value};
    return writeRegister(reg, u16.data(), u16.size());
}

// Explicit template instantiation
template bool Component::writeRegister<uint8_t>(const uint8_t, const uint8_t*,
                                                const size_t);
template bool Component::writeRegister<uint16_t>(const uint16_t, const uint8_t*,
                                                 const size_t);
template bool Component::writeRegister8<uint8_t>(const uint8_t, const uint8_t);
template bool Component::writeRegister8<uint16_t>(const uint16_t,
                                                  const uint8_t);
template bool Component::writeRegister16<uint8_t>(const uint8_t,
                                                  const uint16_t);
template bool Component::writeRegister16<uint16_t>(const uint16_t,
                                                   const uint16_t);

template bool Component::readRegister<uint8_t>(const uint8_t, uint8_t*,
                                               const size_t, const uint32_t);
template bool Component::readRegister<uint16_t>(const uint16_t, uint8_t*,
                                                const size_t, const uint32_t);
template bool Component::readRegister8<uint8_t>(const uint8_t, uint8_t&,
                                                const uint32_t);
template bool Component::readRegister8<uint16_t>(const uint16_t, uint8_t&,
                                                 const uint32_t);
template bool Component::readRegister16<uint8_t>(const uint8_t, uint16_t&,
                                                 const uint32_t);
template bool Component::readRegister16<uint16_t>(const uint16_t, uint16_t&,
                                                  const uint32_t);

std::string Component::debugInfo() const {
    return m5::utility::formatString(
        "[%s]:ID{0X%08x}:ADDR{0X%02x/0X%02x} parent:%u children:%zu",
        deviceName(), identifier(), address(), _adapter->address(), hasParent(),
        childrenSize());
}

}  // namespace unit
}  // namespace m5
