/*!
  @file M5UnitComponent.cpp
  @brief Base class for Unit Component

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
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

Component::Component(const uint8_t addr)
    : _adapter{new Adapter(addr)}, _addr{addr} {
}

Component::Component(Component&& o)
    : _manager{o._manager},
      _adapter{std::move(o._adapter)},
      _order{o._order},
      _cfg{o._cfg},
      _channel{o._channel},
      _addr{o._addr},
      _parent{o._parent},
      _next{o._next},
      _prev{o._prev},
      _child{o._child} {
    o._manager = nullptr;
    o._order   = 0;
    o._cfg     = {};
    o._channel = -1;
    o._addr    = 0x00;
    o._parent = o._next = o._prev = o._child = nullptr;
}

Component& Component::operator=(Component&& o) {
    if (this != &o) {
        _manager = o._manager;
        _adapter = std::move(o._adapter);
        _order   = o._order;
        _cfg     = o._cfg;
        _channel = o._channel;
        _addr    = o._addr;
        _parent  = o._parent;
        _next    = o._next;
        _prev    = o._prev;
        _child   = o._child;

        o._manager = nullptr;
        o._adapter = nullptr;
        o._order   = 0;
        o._cfg     = {};
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
    if (childrenSize() >= _cfg.max_children) {
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
    }
    return nullptr;
}

bool Component::assign(m5::hal::bus::Bus* bus) {
    _adapter.reset(new Adapter(bus, _addr));
    return static_cast<bool>(_adapter);
}

bool Component::assign(TwoWire& wire) {
    _adapter.reset(new Adapter(wire, _addr));
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

#if 0
bool Component::sendCommand(const uint8_t command) {
    return (writeWithTransaction(&command, 1) == m5::hal::error::error_t::OK);
}

bool Component::sendCommand(const uint16_t command) {
    m5::types::big_uint16_t cmd{command};
    return (writeWithTransaction(cmd.data(), cmd.size()) ==
            m5::hal::error::error_t::OK);
}


bool Component::readRegister(const uint16_t addr, uint8_t& result,
                             const uint16_t ms) {


    if (!sendCommand(addr)) {
        M5_LIB_LOGE("Failed to sendCommand");
        return false;
    }
    m5::utility::delay(ms);
    //    return (readWithTransaction(&result, 1) == m5::hal::error::error_t::OK);

    auto r = readWithTransaction(&result, 1);
    M5_LIB_LOGI("readReg8() <%x>: %d", addr, (bool)r);
    return r == m5::hal::error::error_t::OK;

}

bool Component::readRegister(const uint16_t addr, uint16_t& result,
                             const uint16_t ms) {
    if (!sendCommand(addr)) {
        M5_LIB_LOGE("Failed to sendCommand");
        return false;
    }
    m5::utility::delay(ms);

    std::array<uint8_t, 3> rbuf{};
    return readWithTransaction(rbuf.data(), rbuf.size(), [&rbuf, &result]() {
        utility::ReadDataWithCRC16 data(rbuf.data(), 1);
        bool valid = data.valid(0);
        result     = valid ? data.value(0) : 0;
        return valid;
    });
}
#endif

bool Component::sendCommand(const uint16_t command, const uint16_t arg) {
    uint8_t buf[2 + 2 + 1]{};
    new (buf) m5::types::big_uint16_t(command);  // placement new
    m5::types::big_uint16_t* value =
        new (&buf[2]) m5::types::big_uint16_t(arg);  // placement new
    m5::utility::CRC8_Maxim crc;
    buf[4] = crc.get(value->data(), value->size());
    return (writeWithTransaction(buf, m5::stl::size(buf)) ==
            m5::hal::error::error_t::OK);
}

template <typename Reg>
bool Component::readRegister(const Reg reg, uint8_t* rbuf, const size_t len,
                             const uint32_t delayMillis) {
    static_assert(sizeof(reg) <= 2, "overflow");
    static_assert(std::is_integral<Reg>::value && std::is_unsigned<Reg>::value,
                  "Type must be unsigned integer");

    if (!writeRegister(reg)) {
        M5_LIB_LOGE("Failed to sendCommand");
        return false;
    }

    m5::utility::delay(delayMillis);

    return (readWithTransaction(rbuf, len) == m5::hal::error::error_t::OK);
}

template <typename Reg>
bool Component::readRegister8(const Reg reg, uint8_t& result,
                              const uint32_t delayMillis) {
    static_assert(sizeof(reg) <= 2, "overflow");
    static_assert(std::is_integral<Reg>::value && std::is_unsigned<Reg>::value,
                  "Type must be unsigned integer");
    return readRegister(reg, &result, 1, delayMillis);
}

template <typename Reg>
bool Component::readRegister16(const Reg reg, uint16_t& result,
                               const uint32_t delayMillis) {
    static_assert(sizeof(reg) <= 2, "overflow");
    static_assert(std::is_integral<Reg>::value && std::is_unsigned<Reg>::value,
                  "Type must be unsigned integer");

    m5::types::big_uint16_t buf{};
    auto ret = readRegister(reg, buf.data(), buf.size(), delayMillis);
    if (ret) {
        result = buf.get();
    }
    return ret;
}

template <typename Reg>
bool Component::writeRegister(const Reg reg, const uint8_t* buf,
                              const size_t len) {
    static_assert(sizeof(reg) <= 2, "overflow");
    static_assert(std::is_integral<Reg>::value && std::is_unsigned<Reg>::value,
                  "Type must be unsigned integer");

    uint8_t wbuf[sizeof(Reg) + len]{};
    new (wbuf)
        m5::types::big_uint16_t(sizeof(Reg) == 2 ? reg : ((uint16_t)reg) << 8U);
    // Overwrite wbuf[1] if Reg is uint8_t
    memcpy(wbuf + sizeof(Reg), buf, len);

    return (writeWithTransaction(wbuf, sizeof(wbuf)) ==
            m5::hal::error::error_t::OK);
}

template <typename Reg>
bool Component::writeRegister8(const Reg reg, const uint8_t value) {
    return writeRegister(reg, &value, 1);
}

template <typename Reg>
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
