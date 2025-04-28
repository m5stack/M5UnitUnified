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
    M5_LIB_LOGE("Failed to assign");
    return false;
}

bool UnitUnified::add(Component& u, TwoWire& wire)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }

    M5_LIB_LOGD("Add [%s]:0x%02x %zu", u.deviceName(), u.address(), u.childrenSize());

    u._manager = this;
    if (u.assign(wire)) {
        u._order = ++_registerCount;
        _units.emplace_back(&u);
        return add_children(u);
    }
    M5_LIB_LOGE("Failed to assign");
    return false;
}

bool UnitUnified::add(Component& u, m5::unit::Adapter* ad)
{
    if (u.isRegistered()) {
        M5_LIB_LOGW("Already added");
        return false;
    }
    if (!ad) {
        M5_LIB_LOGE("Adapter null");
        return false;
    }

    M5_LIB_LOGD("Add [%s]:0x%02x", u.deviceName(), u.address());

    u._manager = this;
    u._adapter.reset(ad);
    u._order = ++_registerCount;
    _units.emplace_back(&u);

    return add_children(u);
}

bool UnitUnified::add_children(Component& u)
{
    auto it = u.childBegin();
    while (it != u.childEnd()) {
        auto ch = it->channel();

        M5_LIB_LOGV("%s duplicate %u", u.deviceName(), ch);
        auto ad = u.duplicate_adapter(ch);
        if (!ad) {
            M5_LIB_LOGE("Failed to duplicate_adapter() %s:%u", u.deviceName(), ch);
            return false;
        }
        if (!add(*it, ad)) {
            M5_LIB_LOGE("Failed to add %s to %s", it->deviceName(), u.deviceName());
            return false;
        }
        ++it;
    }
    return true;
}

bool UnitUnified::begin(const bool resetAndPlay)
{
    bool result{true};
    M5_LIB_LOGW("Try begin 1st");
    std::for_each(_units.begin(), _units.end(), [&result](Component* c) {
        c->_begun = c->begin();
        M5_LIB_LOGW("  [%s]:%u", c->deviceName(), c->_begun);
        result &= c->_begun;
    });

    // Reinitialize only successful
    // If there is a device with the same address, initialize it again because the previous unit may fall into the above
    // state on subsequent initialization
    if (resetAndPlay) {
        result = true;
        M5_LIB_LOGW("Try begin 2nd");
        std::for_each(_units.begin(), _units.end(), [&result](Component* c) {
            if (c->_begun) {
                c->_begun = c->begin();
                M5_LIB_LOGW("  [%s]:%u", c->deviceName(), c->_begun);
                result &= c->_begun;
            }
        });
    }
    return result;
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
    std::string s = m5::utility::formatString("%*c%s\n", indent * 4, ' ', u->debugInfo().c_str());

    if (u->hasChildren()) {
        s += make_unit_info(u->_child, indent + 1);
    }
    u = u->_next;
    return u ? s += make_unit_info(u, indent) : s;
}

}  // namespace unit
}  // namespace m5
