/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_ADS111x.cpp
  @brief Base class for ADS111x families
*/
#include "unit_ADS111x.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::ads111x;
using namespace m5::unit::ads111x::command;

namespace {
constexpr elapsed_time_t interval_table[] = {
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
    // dupicated[6,7]
    256.f / 32767,
    256.f / 32767,
};

constexpr static Gain gain_table[] = {
    Gain::PGA_6144,
    Gain::PGA_4096,
    Gain::PGA_2048,
    Gain::PGA_1024,
    Gain::PGA_512,
    Gain::PGA_256,
    // duplicated [6,7]
    Gain::PGA_256,
    Gain::PGA_256,
};

}  // namespace

namespace m5 {
namespace unit {
// class UnitADS111x
const char UnitADS111x::name[] = "UnitADS111x";
const types::uid_t UnitADS111x::uid{"UnitADS111x"_mmh3};
const types::uid_t UnitADS111x::attr{0};

bool UnitADS111x::begin() {
    assert(_cfg.stored_size && "stored_size must be greater than zero");
    if (_cfg.stored_size != _data->capacity()) {
        _data.reset(new m5::container::CircularBuffer<Data>(_cfg.stored_size));
        if (!_data) {
            M5_LIB_LOGE("Failed to allocate");
            return false;
        }
    }

    // Check address
    if (!m5::utility::isValidI2CAddress(address())) {
        M5_LIB_LOGE("Invalid address %x", address());
        return false;
    }

    // Processing of the derived class
    if (!on_begin()) {
        M5_LIB_LOGE("Failed to on_begin in derived class");
        return false;
    }

    if (!read_config(_adsCfg)) {
        M5_LIB_LOGE("Failed to get config");
        return false;
    }
    apply_interval(_adsCfg.dr());
    apply_coefficient(_adsCfg.pga());

    return _cfg.start_periodic ? startPeriodicMeasurement()
                               : stopPeriodicMeasurement();
}

void UnitADS111x::update(const bool force) {
    _updated = false;
    if (inPeriodic()) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            // The rate of continuous conversion is equal to the programmeddata
            // rate. Data can be read at any time and always reflect the most
            // recent completed conversion.
            ads111x::Data d{};
            _updated = read_adc_raw(d);
            if (_updated) {
                _latest = at;
                _data->push_back(d);
            }
        }
    }
}

Gain UnitADS111x::gain() const {
    return gain_table[m5::stl::to_underlying(_adsCfg.pga())];
}

bool UnitADS111x::setSamplingRate(ads111x::Sampling rate) {
    Config c{};
    if (read_config(c)) {
        c.dr(rate);
        if (write_config(c)) {
            apply_interval(_adsCfg.dr());
            return true;
        }
    }
    return false;
}

bool UnitADS111x::start_periodic_measurement() {
    if (inPeriodic()) {
        return false;
    }

    Config c{};
    _updated = false;
    if (read_config(c)) {
        c.mode(false);
        _periodic = write_config(c);
        _latest   = 0;
        return _periodic;
    }
    return false;
}

bool UnitADS111x::start_periodic_measurement(const ads111x::Sampling rate) {
    return !inPeriodic() && setSamplingRate(rate) &&
           start_periodic_measurement();
}

bool UnitADS111x::stop_periodic_measurement() {
    Config c{};
    if (read_config(c)) {
        c.mode(true);
        if (write_config(c)) {
            _periodic = false;
            return true;
        }
    }
    return false;
}

bool UnitADS111x::measureSingleshot(ads111x::Data& d,
                                    const uint32_t timeoutMillis) {
    if (inPeriodic()) {
        M5_LIB_LOGW("Periodic measurements are running");
        return false;
    }

    if (start_single_measurement()) {
        auto timeout_at = m5::utility::millis() + timeoutMillis;
        bool done{};
        do {
            done = !in_conversion();
        } while (!done && m5::utility::millis() <= timeout_at);
        if (done) {
            return read_adc_raw(d);
        }
    }
    return false;
}

bool UnitADS111x::start_single_measurement() {
    if (inPeriodic()) {
        M5_LIB_LOGW("Periodic measurements are running");
        return false;
    }

    Config c{};
    if (read_config(c)) {
        // This bit determines the operational status of the device. OS can
        // only be written when in power-down state and has no effect when a
        // conversion is ongoing.
        c.os(true);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::in_conversion() {
    Config c{};
    return read_config(c) && !c.os();
}

bool UnitADS111x::read_adc_raw(ads111x::Data& d) {
    if (readRegister16(CONVERSION_REG, d.raw, 0)) {
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
        if (read_config(_adsCfg) && _adsCfg.mode()) {
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

bool UnitADS111x::readThreshould(int16_t& high, int16_t& low) {
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
bool UnitADS111x::read_config(ads111x::Config& c) {
    return readRegister16(CONFIG_REG, c.value, 0);
}

bool UnitADS111x::write_config(const ads111x::Config& c) {
    if (writeRegister16(CONFIG_REG, c.value)) {
        _adsCfg = c;
        return true;
    }
    return false;
}

void UnitADS111x::apply_interval(const ads111x::Sampling rate) {
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
    if (read_config(c)) {
        c.mux(mux);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_gain(const ads111x::Gain gain) {
    Config c{};
    if (read_config(c)) {
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
    if (read_config(c)) {
        c.comp_mode(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_comparator_polarity(const bool b) {
    Config c{};
    if (read_config(c)) {
        c.comp_pol(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_latching_comparator(const bool b) {
    Config c{};
    if (read_config(c)) {
        c.comp_lat(b);
        return write_config(c);
    }
    return false;
}

bool UnitADS111x::set_comparator_queue(const ads111x::ComparatorQueue q) {
    Config c{};
    if (read_config(c)) {
        c.comp_que(q);
        return write_config(c);
    }
    return false;
}
}  // namespace unit
}  // namespace m5
