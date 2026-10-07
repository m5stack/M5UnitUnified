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

// types.hpp defines elapsed_time_t on its own so that the public headers do not depend on M5Utility.
// Keep it the same type as m5::utility::elapsed_time_t (the return type of m5::utility::millis()).
static_assert(std::is_same<elapsed_time_t, m5::utility::elapsed_time_t>::value,
              "m5::unit::types::elapsed_time_t must match m5::utility::elapsed_time_t");

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

bool Component::add(Component& c, const int16_t ch16)
{
    if (childrenSize() >= _component_cfg.max_children) {
        M5_LIB_LOGE("Can't connect any more");
        return false;
    }
    if (ch16 < 0 || ch16 > 255) {
        M5_LIB_LOGE("Invalid channel %d", ch16);
        return false;
    }

    const uint8_t ch = static_cast<uint8_t>(ch16);
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
    if (!bus) {
        return false;
    }

    // Bus type and unit access capability must agree
    switch (bus->getBusType()) {
        case m5::hal::types::BusType::I2C:
            if (!canAccessI2C() || !_addr) return false;
            _adapter = std::make_shared<AdapterI2C>(bus, _addr, _component_cfg.clock);
            return static_cast<bool>(_adapter);

        case m5::hal::types::BusType::SPI:
            if (!canAccessSPI()) return false;
            M5_LIB_LOGE("M5HAL SPI bus assign is not yet implemented");
            return false;

        case m5::hal::types::BusType::UART:
            if (!canAccessUART()) return false;
            M5_LIB_LOGE("M5HAL UART bus assign is not yet implemented");
            return false;

        case m5::hal::types::BusType::GPIO:
            if (!canAccessGPIO()) return false;
            M5_LIB_LOGE("M5HAL GPIO bus assign is not yet implemented");
            return false;

        default:
            return false;
    }
}

#if defined(ARDUINO)
bool Component::assign(TwoWire& wire)
{
    if (canAccessI2C() && _addr) {
        _adapter = std::make_shared<AdapterI2C>(wire, _addr, _component_cfg.clock);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#endif

bool Component::assign(m5::I2C_Class& i2c)
{
    if (canAccessI2C() && _addr) {
        _adapter = std::make_shared<AdapterI2C>(i2c, _addr, _component_cfg.clock);
        return static_cast<bool>(_adapter);
    }
    return false;
}

#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
bool Component::assign(i2c_master_bus_handle_t bus)
{
    if (canAccessI2C() && _addr && bus) {
        _adapter = std::make_shared<AdapterI2C>(bus, _addr, _component_cfg.clock);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#elif defined(ESP_PLATFORM)
bool Component::assign(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl)
{
    if (canAccessI2C() && _addr) {
        _adapter = std::make_shared<AdapterI2C>(port, sda, scl, _addr, _component_cfg.clock);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#endif

#if defined(ESP_PLATFORM)
bool Component::assign(const int8_t rx_pin, const int8_t tx_pin)
{
    if (canAccessGPIO()) {
        _adapter = std::make_shared<AdapterGPIO>(rx_pin, tx_pin);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#endif

#if defined(ARDUINO)
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
        // address() is reused as the CS GPIO number on SPI units (see CapST25R3916 etc.)
        _adapter = std::make_shared<AdapterSPI>(spi, settings, static_cast<gpio_num_t>(address()) /* CS */);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#endif

#if defined(ESP_PLATFORM)
bool Component::assign(const uart_port_t uart_num)
{
    if (canAccessUART() && uart_is_driver_installed(uart_num)) {
        _adapter = std::make_shared<AdapterUART>(uart_num);
        return static_cast<bool>(_adapter);
    }
    return false;
}

bool Component::assign(spi_device_handle_t handle, const gpio_num_t cs)
{
    if (canAccessSPI()) {
        // If cs is omitted (GPIO_NUM_NC), use address() as the CS pin (same convention as Arduino SPI).
        const gpio_num_t actual_cs = (cs == GPIO_NUM_NC) ? static_cast<gpio_num_t>(address()) : cs;
        _adapter                   = std::make_shared<AdapterSPI>(handle, actual_cs);
        return static_cast<bool>(_adapter);
    }
    return false;
}
#endif

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
        result = static_cast<uint32_t>(tmp[0 + 3 * endian]) | (static_cast<uint32_t>(tmp[1 + endian]) << 8) |
                 (static_cast<uint32_t>(tmp[2 - endian]) << 16) | (static_cast<uint32_t>(tmp[3 - 3 * endian]) << 24);
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

#if defined(ESP_PLATFORM)
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

bool Component::readAnalogMilliVoltsRX(uint32_t& mv)
{
    return adapter()->readAnalogMilliVoltsRX(mv) == m5::hal::error::error_t::OK;
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

bool Component::readAnalogMilliVoltsTX(uint32_t& mv)
{
    return adapter()->readAnalogMilliVoltsTX(mv) == m5::hal::error::error_t::OK;
}

bool Component::pulseInTX(uint32_t& duration, const int state, const uint32_t timeout_us)
{
    return adapter()->pulseInTX(duration, state, timeout_us) == m5::hal::error::error_t::OK;
}
#endif  // ESP_PLATFORM

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

namespace {
const char* i2c_impl_name(const AdapterI2C::ImplType t)
{
    switch (t) {
        case AdapterI2C::ImplType::TwoWire:
            return "TwoWire";
        case AdapterI2C::ImplType::Bus:
            return "M5HAL";
        case AdapterI2C::ImplType::I2CClass:
            return "I2C_Class";
#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
        case AdapterI2C::ImplType::ESPIDFMasterBus:
            return "ESP-IDF";
#elif defined(ESP_PLATFORM)
        case AdapterI2C::ImplType::ESPIDFLegacyBus:
            return "ESP-IDF(legacy)";
#endif
        default:
            return "Unknown";
    }
}
}  // namespace

// e.g. "#2 UnitPuzzle{0x9C1E33D5}  UnitPbHub#1 ch:0  children:1/1"
//      "#1 UnitPbHub{0x2B4F7A10}  I2C TwoWire(sda:21 scl:22) 0x61  children:1/6"
std::string Component::debugInfo() const
{
    std::string conn{};
    const auto* i2c = (_adapter->type() == Adapter::Type::I2C) ? asAdapter<AdapterI2C>(Adapter::Type::I2C) : nullptr;
    if (_parent) {
        // Where on the parent (hub channel). A child with its own I2C address (e.g. behind PaHub) shows it,
        // one reached through the parent's address (e.g. PbHub channel) does not
        conn = m5::utility::formatString("%s#%u ch:%u", _parent->deviceName(), _parent->order(), channel());
        const auto* parent_i2c = _parent->asAdapter<AdapterI2C>(Adapter::Type::I2C);
        if (i2c && (!parent_i2c || i2c->address() != parent_i2c->address())) {
            conn += m5::utility::formatString(" 0x%02X", i2c->address());
        }
    } else {
        switch (_adapter->type()) {
            case Adapter::Type::I2C: {
                const auto* impl = i2c->impl();
                conn             = m5::utility::formatString("I2C %s", i2c_impl_name(impl->implType()));
                if (impl->sda() >= 0 && impl->scl() >= 0) {
                    conn += m5::utility::formatString("(sda:%d scl:%d)", impl->sda(), impl->scl());
                }
                conn += m5::utility::formatString(" 0x%02X", i2c->address());
            } break;
#if defined(ESP_PLATFORM)
            case Adapter::Type::GPIO: {
                const auto* gpio = asAdapter<AdapterGPIO>(Adapter::Type::GPIO);
                conn             = m5::utility::formatString("GPIO rx:%d tx:%d", gpio->rx_pin(), gpio->tx_pin());
            } break;
            case Adapter::Type::SPI:
                conn = m5::utility::formatString("SPI cs:%d", asAdapter<AdapterSPI>(Adapter::Type::SPI)->cs_pin());
                break;
#endif
            case Adapter::Type::UART:
                conn = "UART";
                break;
            default:
                conn = "(not connected)";
                break;
        }
    }
    return m5::utility::formatString("#%u %s{0x%08X}  %s  children:%zu/%u", order(), deviceName(),
                                     static_cast<unsigned>(identifier()), conn.c_str(), childrenSize(),
                                     _component_cfg.max_children);
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
