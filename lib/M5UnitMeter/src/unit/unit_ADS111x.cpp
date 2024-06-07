/*!
  @file unit_ADS111x.cpp
  @brief ADS111x Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_ADS111x.hpp"
#include <M5Utility.hpp>

namespace {
constexpr unsigned long interval_table[] = {
    1000UL / 8,   1000UL / 16,  1000UL / 32,  1000UL / 64,
    1000UL / 128, 1000UL / 250, 1000UL / 475, 1000UL / 860,
};

constexpr float coefficient_table[] = {
    6.144f / 32767,
    4.096f / 32767,
    2.048f / 32767,
    1.024f / 32767,
    0.512f / 32767,
    0.256f / 32767,
    // dupicated
    0.256f / 32767,
    0.256f / 32767,
};

}  // namespace

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

    if (!get_config(_adsCfg)) {
        M5_LIB_LOGE("Failed to get config");
        return false;
    }

    M5_LIB_LOGE("==== %x", _adsCfg.value);

    M5_LIB_LOGE("<<< %d", (int)m5::stl::to_underlying(_adsCfg.dr()));

    _interval    = interval_table[m5::stl::to_underlying(_adsCfg.dr())];
    _coefficient = coefficient_table[m5::stl::to_underlying(_adsCfg.pga())];

    M5_LIB_LOGI("interval:%u", _interval);

    return stopPeriodicMeasurement();
}

void UnitADS111x::update() {
    if (inPeriodic()) {
        unsigned long at{m5::utility::millis()};
        if (!_latest || at >= _latest + _interval) {
            _updated = read_ads_raw(_value);
            if (_updated) {
                _latest = at;
            }
        } else {
            _updated = false;
        }
    }
}

Gain UnitADS111x::gain() const {
    constexpr static Gain table[] = {
        Gain::PGA_6144,
        Gain::PGA_4096,
        Gain::PGA_2048,
        Gain::PGA_1024,
        Gain::PGA_512,
        Gain::PGA_256,
        // duplicated
        Gain::PGA_256,
        Gain::PGA_256,
    };
    return table[m5::stl::to_underlying(_adsCfg.pga())];
}

bool UnitADS111x::setRate(ads111x::Rate rate) {
    Config c{};
    if (get_config(c)) {
        c.dr(rate);
        if (write_config(c)) {
            _interval = interval_table[m5::stl::to_underlying(_adsCfg.dr())];
            return true;
        }
    }
    return false;
}

bool UnitADS111x::startPeriodicMeasurement() {
    Config c{};
    _updated = false;
    if (get_config(c)) {
        c.mode(false);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::stopPeriodicMeasurement() {
    Config c{};
    _updated = false;
    if (get_config(c)) {
        c.mode(true);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::startSingleMeasurement() {
    if (inPeriodic()) {
        M5_LIB_LOGW("Periodic measurements are running");
        return false;
    }
    _updated = false;

    Config c{};
    if (get_config(c)) {
        // This bit determines the operational status of the device. OS can only
        // be written when in power-down state and has no effect when a
        // conversion is ongoing.
        c.os(true);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::inConversion() {
    Config c{};
    return get_config(c) && !c.os();
}

bool UnitADS111x::readSingleMeasurement(uint16_t& raw,
                                        const uint32_t timeoutMillis) {
    if (!inPeriodic() && startSingleMeasurement()) {
        auto timeout_at = m5::utility::millis() + timeoutMillis;
        bool done{};
        do {
            done = !inConversion();
        } while (!done && m5::utility::millis() <= timeout_at);
        if (done) {
            return getAdcRaw(raw);
        }
    }
    return false;
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
        if (get_config(_adsCfg) && _adsCfg.mode()) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (!done && m5::utility::millis() <= timeout_at);

    if (done) {
        _interval = interval_table[m5::stl::to_underlying(_adsCfg.dr())];
    }
    return done;
}

bool UnitADS111x::getThreshould(int16_t& high, int16_t& low) {
    uint16_t hh{}, ll{};
    if (readRegister16(HIGH_THRESHOLD_REG, hh, 0) &&
        readRegister16(LOW_THRESHOLD_REG, ll, 0)) {
        high = hh;
        low  = ll;
        return true;
    }
    return false;
}

bool UnitADS111x::setThreshould(const int16_t high, const int16_t low) {
    if (high <= low) {
        M5_LIB_LOGW("high must be greater than low");
        return false;
    }
    return writeRegister16(HIGH_THRESHOLD_REG, (uint16_t)high) &&
           writeRegister16(LOW_THRESHOLD_REG, (uint16_t)low);
}

//
bool UnitADS111x::get_config(ads111x::Config& c) {
    return readRegister16(CONFIG_REG, c.value, 0);
}

bool UnitADS111x::write_config(const ads111x::Config& c) {
    if (writeRegister16(CONFIG_REG, c.value)) {
        _adsCfg = c;
        return true;
    }
    return false;
}

bool UnitADS111x::set_multiplexer(const ads111x::Mux mux) {
    Config c{};
    if (get_config(c)) {
        c.mux(mux);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_gain(const ads111x::Gain gain) {
    Config c{};
    if (get_config(c)) {
        c.pga(gain);
        if (write_config(c)) {
            _coefficient =
                coefficient_table[m5::stl::to_underlying(_adsCfg.pga())];
            return true;
        }
    }
    return false;
}

bool UnitADS111x::set_comparator_mode(const bool b) {
    Config c{};
    if (get_config(c)) {
        c.comp_mode(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_comparator_polarity(const bool b) {
    Config c{};
    if (get_config(c)) {
        c.comp_pol(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_latching_comparator(const bool b) {
    Config c{};
    if (get_config(c)) {
        c.comp_lat(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_comparator_queue(const ads111x::ComparatorQueue q) {
    Config c{};
    if (get_config(c)) {
        c.comp_que(q);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::read_ads_raw(uint16_t& raw) {
    //    return !inConversion() && getAdcRaw(raw);
    return getAdcRaw(raw);
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
