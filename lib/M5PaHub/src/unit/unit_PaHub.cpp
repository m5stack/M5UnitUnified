/*!
  @file unit_PaHub.cpp
  @brief PaHub Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_PaHub.hpp"
#include "m5_unit_component/adapter.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

// class UnitPaHub
const char UnitPaHub::name[] = "UnitPaHub";
const types::uid_t UnitPaHub::uid{"UnitPaHub"_mmh3};
const types::uid_t UnitPaHub::attr{0};

UnitPaHub::UnitPaHub(const uint8_t addr) : Component(addr) {
    auto cfg         = component_config();
    cfg.max_children = MAX_CHANNEL;
    component_config(cfg);
}

UnitPaHub::UnitPaHub(UnitPaHub&& o)
    : Component(std::move(o)), _adapters(std::move(o._adapters)) {
}

UnitPaHub& UnitPaHub::operator=(UnitPaHub&& o) {
    if (this != &o) {
        Component::operator=(std::move(o));
        _adapters = std::move(o._adapters);
    }
    return *this;
}

Adapter* UnitPaHub::ensure_adapter(const uint8_t ch) {
    if (ch >= _adapters.size()) {
        M5_LIB_LOGE("Invalid channel %u", ch);
        return nullptr;
    }

    auto unit = child(ch);
    if (!unit) {
        M5_LIB_LOGE("Not exists unit %u", ch);
        return nullptr;
    }

    auto& ad = _adapters[ch];
    if (!ad) {
        ad.reset(_adapter->duplicate(unit->address()));
        // ad.reset(new PaHubAdapter(unit->address()));
    }
    return ad.get();
}

m5::hal::error::error_t UnitPaHub::select_channel(const uint8_t ch) {
    M5_LIB_LOGV("Try current to %u / %u", ch, _current);

    if (ch != _current && ch < MAX_CHANNEL) {
        _current       = 0;
        uint8_t buf[1] = {static_cast<uint8_t>((1U << ch) & 0xFF)};
        auto ret       = writeWithTransaction(buf, 1);
        if (ret == m5::hal::error::error_t::OK) {
            _current = ch;
        }
        return ret;
    }
    return m5::hal::error::error_t::UNKNOWN_ERROR;
}

}  // namespace unit
}  // namespace m5
