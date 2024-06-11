/*!
  @file unit_ADS1115_with_EEPROM.cpp
  @brief Base class for Ameter and Vmeter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_ADS1115_with_EEPROM.hpp"
#include <M5Utility.hpp>

#if 0
//A
======== [0]:1   -1,-1
======== [1]:1   -1,-1
======== [2]:1   -1,-1
======== [3]:1   -1,-1
======== [4]:1   6400,-6423
======== [5]:1   -1,-1
//V
======== [0]:1   -1,-1
======== [1]:1   7641,7613
======== [2]:1   -1,-1
======== [3]:1   -1,-1
======== [4]:1   5094,5073
======== [5]:1   -1,-1
#endif

namespace m5 {
namespace unit {

using namespace m5::utility::mmh3;
using namespace ads111x;
using namespace ads111x::command;

const char UnitADS1115WithEEPROM::name[] = "UnitADS1115WithEEPROM";
const types::uid_t UnitADS1115WithEEPROM::uid{"UnitADS1115WithEEPROM"_mmh3};
const types::uid_t UnitADS1115WithEEPROM::attr{0};

bool UnitADS1115WithEEPROM::assign(m5::hal::bus::Bus* bus) {
    if (Component::assign(bus)) {
        if (_eepromAddr) {
            _adapterEEPROM.reset(new Adapter(bus, _eepromAddr));
        }
        return static_cast<bool>(_adapterEEPROM);
    }
    return false;
}

bool UnitADS1115WithEEPROM::assign(TwoWire& wire) {
    if (Component::assign(wire)) {
        if (_eepromAddr) {
            _adapterEEPROM.reset(new Adapter(wire, _eepromAddr));
        }
        return static_cast<bool>(_adapterEEPROM);
    }
    return false;
}

bool UnitADS1115WithEEPROM::begin() {
    if (!UnitADS1115::begin()) {
        return false;
    }

    Gain table[] = {
        Gain::PGA_6144, Gain::PGA_4096, Gain::PGA_2048,
        Gain::PGA_1024, Gain::PGA_512,  Gain::PGA_256,
    };
    int idx{};
    for (auto&& e : table) {
        if (!read_calibration(e, _calibration[idx].hope,
                              _calibration[idx].actual)) {
            M5_LIB_LOGE("Failed ti read calibration data");
            return false;
        }
        M5_LIB_LOGV("Calibration[%u]: %d,%d", e, _calibration[idx].hope,
                    _calibration[idx].actual);
        ++idx;
    }
    apply_calibration(_adsCfg.pga());
    return true;
}

bool UnitADS1115WithEEPROM::setGain(const ads111x::Gain gain) {
    if (UnitADS1115::setGain(gain)) {
        apply_calibration(gain);
        return true;
    }
    return false;
}

bool UnitADS1115WithEEPROM::read_calibration(const Gain gain, int16_t& hope,
                                             int16_t& actual) {
    uint8_t reg = 0xD0 + m5::stl::to_underlying(gain) * 8;
    uint8_t buf[8]{};

    if (_adapterEEPROM->writeWithTransaction(reg, nullptr, 0U) !=
        m5::hal::error::error_t::OK) {
        M5_LIB_LOGE("Failed to write");
        return false;
    }
    if (_adapterEEPROM->readWithTransaction(buf, sizeof(buf)) !=
        m5::hal::error::error_t::OK) {
        return false;
    }

    uint8_t xorchk{};
    for (int_fast8_t i = 0; i < 5; ++i) {
        xorchk ^= buf[i];
    }
    if (xorchk != buf[5]) {
        return false;
    }

    m5::types::big_uint16_t hh(buf[1], buf[2]);
    m5::types::big_uint16_t aa(buf[3], buf[4]);
    hope   = (int16_t)hh.get();
    actual = (int16_t)aa.get();

    return true;
}

void UnitADS1115WithEEPROM::apply_calibration(const Gain gain) {
    auto idx = m5::stl::to_underlying(gain);
    _calibrationFactor =
        (idx < m5::stl::size(_calibration) && _calibration[idx].actual)
            ? (float)_calibration[idx].hope / _calibration[idx].actual
            : 1.0f;
}

}  // namespace unit
}  // namespace m5
