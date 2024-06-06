/*!
  @file unit_ADS111x.cpp
  @brief ADS111x Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_ADS111x.hpp"
#include <M5Utility.hpp>

namespace {
const uint32_t interval_table[] = {
    1000 / 8,   1000 / 16,  1000 / 64,  1000 / 128,
    1000 / 250, 1000 / 475, 1000 / 860,
};

}

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

using namespace ads111x;
using namespace ads111x::command;

// class UnitADS111x
const char UnitADS111x::name[] = "UnitADS111x";
const types::uid_t UnitADS111x::uid{"UnitADS111x"_mmh3};
const types::uid_t UnitADS111x::attr{0};

bool UnitADS111x::begin() {
    Config cfg;

    if (address() & 0x80) {
        M5_LIB_LOGE("Invalid address");
        return false;
    }
    return apply_config() && stopPeriodicMeasurement();
}

void UnitADS111x::update() {
}

bool UnitADS111x::setMultiplexer(const ads111x::Mux mux) {
    if (get_config()) {
        _adsCfg.mux(mux);
        return writeRegister16(CONFIG_REG, _adsCfg.value);
    }
    return false;
}

bool UnitADS111x::setGain(const ads111x::Gain gain) {
    if (get_config()) {
        _adsCfg.pga(gain);
        return writeRegister16(CONFIG_REG, _adsCfg.value);
    }
    return false;
}

bool UnitADS111x::setRate(ads111x::Rate rate) {
    if (get_config()) {
        _adsCfg.dr(rate);
        return writeRegister16(CONFIG_REG, _adsCfg.value);
    }
    return false;
}

bool UnitADS111x::setComparatorQueue(const ads111x::ComparatorQueue c) {
    if (get_config()) {
        _adsCfg.comp_que(c);
        return writeRegister16(CONFIG_REG, _adsCfg.value);
    }
    return false;
}

bool UnitADS111x::startPeriodicMeasurement() {
    if (get_config()) {
        _adsCfg.mode(false);
        _periodic = writeRegister16(CONFIG_REG, _adsCfg.value);
        return _periodic;
    }
    return false;
}

bool UnitADS111x::stopPeriodicMeasurement() {
    if (get_config()) {
        _adsCfg.mode(true);
        if (writeRegister16(CONFIG_REG, _adsCfg.value)) {
            _periodic = false;
            return true;
        }
    }
    return false;
}

bool UnitADS111x::startSingleMeasurement() {
    if (inPeriodic()) {
        M5_LIB_LOGW("Periodic measurements are running");
        return false;
    }

    if (get_config()) {
        // This bit determines the operational status of the device. OS can only
        // be written when in power-down state and has no effect when a
        // conversion is ongoing.
        _adsCfg.os(true);
        return writeRegister16(CONFIG_REG, _adsCfg.value);
    }
    return false;
}

bool UnitADS111x::inConversion() {
    return get_config() && !_adsCfg.os();
}

bool UnitADS111x::getConversion(uint16_t& cv, const uint32_t millis) {
    if (!inPeriodic() && startSingleMeasurement()) {
        auto timeout_at = m5::utility::millis() + millis;
        bool done{};
        do {
            done = !inConversion();
        } while (!done && m5::utility::millis() <= timeout_at);
        if (!done) {
            return false;
        }
    }
    return getAdcRaw(cv);
}

bool UnitADS111x::getAdcRaw(uint16_t& raw) {
    return readRegister16(CONVERSION_REG, raw, 0);
}

bool UnitADS111x::generalReset() {
    uint8_t cmd{0x06};  // reset command
    generalCall(&cmd, 1);

    auto timeout_at = m5::utility::millis() + 10;
    bool done{};
    Config c{};
    do {
        // power-down mode?
        if (get_config() && _adsCfg.mode()) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (!done && m5::utility::millis() <= timeout_at);
    return done && apply_config();
}

bool UnitADS111x::get_config() {
    return readRegister16(CONFIG_REG, _adsCfg.value, 0);
}

bool UnitADS111x::apply_config() {
    _interval = interval_table[m5::stl::to_underlying(_adsCfg.dr())];
    return true;
}

// class UnitADS1113
const char UnitADS1113::name[] = "UnitADS1113";
const types::uid_t UnitADS1113::uid{"UnitADS1113"_mmh3};
const types::uid_t UnitADS1113::attr{0};

// class UnitADS1114
const char UnitADS1114::name[] = "UnitADS1114";
const types::uid_t UnitADS1114::uid{"UnitADS1114"_mmh3};
const types::uid_t UnitADS1114::attr{0};

// class UnitADS1115
const char UnitADS1115::name[] = "UnitADS1115";
const types::uid_t UnitADS1115::uid{"UnitADS1115"_mmh3};
const types::uid_t UnitADS1115::attr{0};

}  // namespace unit
}  // namespace m5
