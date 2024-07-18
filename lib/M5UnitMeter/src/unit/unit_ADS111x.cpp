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
    6144.f / 32767,
    4096.f / 32767,
    2048.f / 32767,
    1024.f / 32767,
    512.f / 32767,
    256.f / 32767,
    // dupicated
    256.f / 32767,
    256.f / 32767,
};

}  // namespace

namespace m5 {
namespace unit {

using namespace m5::utility::mmh3;
using namespace ads111x;
using namespace ads111x::command;

// class UnitADS111x
const char UnitADS111x::name[] = "UnitADS111x";
const types::uid_t UnitADS111x::uid{"UnitADS111x"_mmh3};
const types::uid_t UnitADS111x::attr{0};

bool UnitADS111x::begin() {
    auto cfg = config();

    if (address() & 0x80) {
        M5_LIB_LOGE("Invalid address");
        return false;
    }

    if (!on_begin()) {
        M5_LIB_LOGE("Failed to set settings");
        return false;
    }

    if (!get_config(_adsCfg)) {
        M5_LIB_LOGE("Failed to get config");
        return false;
    }
    apply_interval(_adsCfg.dr());
    apply_coefficient(_adsCfg.pga());

    return cfg.periodic ? startPeriodicMeasurement()
                        : stopPeriodicMeasurement();
}

void UnitADS111x::update(const bool force) {
    _updated = false;
    if (inPeriodic()) {
        unsigned long at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            // The rate of continuous conversion is equal to the programmeddata
            // rate. Data can be read at any time and always reflect the most
            // recent completed conversion.
            _updated = getAdcRaw(_value);
            if (_updated) {
                _latest = at;
            }
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
            apply_interval(_adsCfg.dr());
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

bool UnitADS111x::readSingleMeasurement(int16_t& raw,
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

bool UnitADS111x::getAdcRaw(int16_t& raw) {
    uint16_t value{};
    if (readRegister16(CONVERSION_REG, value, 0)) {
        raw = (int16_t)value;
        return true;
    }
    return false;
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
        apply_interval(_adsCfg.dr());
        apply_coefficient(_adsCfg.pga());
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

void UnitADS111x::apply_interval(const ads111x::Rate rate) {
    auto idx = m5::stl::to_underlying(rate);
    assert(idx < m5::stl::size(interval_table) && "Illegal value");
    _interval = interval_table[idx];
}

void UnitADS111x::apply_coefficient(const ads111x::Gain gain) {
    auto idx = m5::stl::to_underlying(gain);
    assert(idx < m5::stl::size(coefficient_table) && "Illegal value");
    _coefficient = coefficient_table[idx];
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
            apply_coefficient(_adsCfg.pga());
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

// class UnitADS1113
const char UnitADS1113::name[] = "UnitADS1113";
const types::uid_t UnitADS1113::uid{"UnitADS1113"_mmh3};
const types::uid_t UnitADS1113::attr{0};
bool UnitADS1113::on_begin() {
    M5_LIB_LOGD("mux, gain, and comp_que  not support");
    return setRate(_cfg.rate);
}

// class UnitADS1114
const char UnitADS1114::name[] = "UnitADS1114";
const types::uid_t UnitADS1114::uid{"UnitADS1114"_mmh3};
const types::uid_t UnitADS1114::attr{0};
bool UnitADS1114::on_begin() {
    M5_LIB_LOGD("mux is not support");
    return setRate(_cfg.rate) && setGain(_cfg.gain) &&
           setComparatorQueue(_cfg.comp_que);
}

// class UnitADS1115
const char UnitADS1115::name[] = "UnitADS1115";
const types::uid_t UnitADS1115::uid{"UnitADS1115"_mmh3};
const types::uid_t UnitADS1115::attr{0};
bool UnitADS1115::on_begin() {
    return setRate(_cfg.rate) && setMultiplexer(_cfg.mux) &&
           setGain(_cfg.gain) && setComparatorQueue(_cfg.comp_que);
}

}  // namespace unit
}  // namespace m5
