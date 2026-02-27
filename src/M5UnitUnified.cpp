/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnified.cpp
  @brief class UnitUnified
*/
#include "M5UnitUnified.hpp"
#include <M5Utility.hpp>

namespace m5 {
namespace unit {

uint32_t UnitUnified::_registerCount{0};

bool UnitUnified::add(Component& u, m5::hal::bus::Bus* bus)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }
    if (!bus) {
        M5_LIB_LOGE("Bus null");
        return false;
    }

    M5_LIB_LOGD("Add [%s]:0x%02x", u.deviceName(), u.address());

    u._manager = this;
    if (u.assign(bus)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessI2C());
    return false;
}

bool UnitUnified::add(Component& u, TwoWire& wire)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s] addr:%02x children:%zu", u.deviceName(), u.address(), u.childrenSize());

    u._manager = this;
    if (u.assign(wire)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessI2C());
    return false;
}

bool UnitUnified::add(Component& u, m5::I2C_Class& i2c)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s] addr:%02x children:%zu", u.deviceName(), u.address(), u.childrenSize());

    u._manager = this;
    if (u.assign(i2c)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessI2C());
    return false;
}

bool UnitUnified::add(Component& u, const int8_t rx_pin, const int8_t tx_pin)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s] rx:%d tx:%d %zu", u.deviceName(), rx_pin, tx_pin, u.childrenSize());

    u._manager = this;
    if (u.assign(rx_pin, tx_pin)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessGPIO());
    return false;
}

bool UnitUnified::add(Component& u, HardwareSerial& serial)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s] addr:%02x children:%zu", u.deviceName(), u.address(), u.childrenSize());

    u._manager = this;
    if (u.assign(serial)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessI2C());
    return false;
}

bool UnitUnified::add(Component& u, SPIClass& spi, const SPISettings& settings)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s] addr:%02x children:%zu", u.deviceName(), u.address(), u.childrenSize());

    u._manager = this;
    if (u.assign(spi, settings)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign %s:%u", u.deviceName(), u.canAccessI2C());
    return false;
}

// Add children if exists (iterative to avoid stack overflow)
bool UnitUnified::add_children(Component& u)
{
    std::vector<Component*> stack;
    stack.reserve(8);
    stack.push_back(&u);

    while (!stack.empty()) {
        Component* parent = stack.back();
        stack.pop_back();

        for (auto it = parent->childBegin(); it != parent->childEnd(); ++it) {
            auto ch = it->channel();

            M5_LIB_LOGV("%s child:%s channel:%u", parent->deviceName(), it->deviceName(), ch);
            if (it->isRegistered()) {
                M5_LIB_LOGE("Already registered %s", it->deviceName());
                return false;
            }
            it->_manager = this;
            it->_adapter = parent->ensure_adapter(ch);
            M5_LIB_LOGD("  Shared:%u %u", parent->_adapter.use_count(), it->_adapter.use_count());
            it->_order = ++_registerCount;
            _units.emplace_back(&*it);

            if (it->hasChildren()) {
                stack.push_back(&*it);
            }
        }
    }
    return true;
}

bool UnitUnified::begin()
{
    return !std::any_of(_units.begin(), _units.end(), [](Component* c) {
        M5_LIB_LOGV("Try begin:%s", c->deviceName());
        bool ret = c->_begun = c->begin();
        if (!ret) {
            M5_LIB_LOGE("Failed to begin: %s", c->debugInfo().c_str());
        }
        return !ret;
    });
}

void UnitUnified::update(const bool force)
{
    // Order of registration
    for (auto&& u : _units) {
        if (!u->_component_cfg.self_update && u->_begun) {
            u->update(force);
        }
    }
}

std::string UnitUnified::debugInfo() const
{
    std::string s = m5::utility::formatString("\nM5UnitUnified: %zu units\n", _units.size());
    for (auto&& u : _units) {
        if (!u->hasParent()) {
            s += make_unit_info(u, 0);
        }
    }
    return m5::utility::trim(s);
}

std::string UnitUnified::make_unit_info(const Component* u, const uint8_t indent) const
{
    std::string s;
    s.reserve(256);

    struct StackEntry {
        const Component* node;
        uint8_t indent;
    };
    std::vector<StackEntry> stack;
    stack.reserve(16);
    stack.push_back({u, indent});

    while (!stack.empty()) {
        auto entry = stack.back();
        stack.pop_back();

        s += m5::utility::formatString("%*c%s\n", entry.indent * 4, ' ', entry.node->debugInfo().c_str());

        // Push sibling first (processed later = output later)
        if (entry.node->_next) {
            stack.push_back({entry.node->_next, entry.indent});
        }
        // Push child second (processed first = output first)
        if (entry.node->hasChildren()) {
            stack.push_back({entry.node->_child, static_cast<uint8_t>(entry.indent + 1)});
        }
    }
    return s;
}

}  // namespace unit
}  // namespace m5
