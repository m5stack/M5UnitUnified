/*!
  @file unit_PAJ7620U2.cpp
  @brief PAJ7620U2Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_PAJ7620U2.hpp"
#include <M5Utility.hpp>
#include <array>

namespace {
constexpr uint16_t chip_id{0x7620};
constexpr uint8_t register_for_initialize[][2] = {
    {0xEF, 0x00}, {0x37, 0x07}, {0x38, 0x17}, {0x39, 0x06}, {0x42, 0x01},
    {0x46, 0x2D}, {0x47, 0x0F}, {0x48, 0x3C}, {0x49, 0x00}, {0x4A, 0x1E},
    {0x4C, 0x20}, {0x51, 0x10}, {0x5E, 0x10}, {0x60, 0x27}, {0x80, 0x42},
    {0x81, 0x44}, {0x82, 0x04}, {0x8B, 0x01}, {0x90, 0x06}, {0x95, 0x0A},
    {0x96, 0x0C}, {0x97, 0x05}, {0x9A, 0x14}, {0x9C, 0x3F}, {0xA5, 0x19},
    {0xCC, 0x19}, {0xCD, 0x0B}, {0xCE, 0x13}, {0xCF, 0x64}, {0xD0, 0x21},
    {0xEF, 0x01}, {0x02, 0x0F}, {0x03, 0x10}, {0x04, 0x02}, {0x25, 0x01},
    {0x27, 0x39}, {0x28, 0x7F}, {0x29, 0x08}, {0x3E, 0xFF}, {0x5E, 0x3D},
    {0x65, 0x96}, {0x67, 0x97}, {0x69, 0xCD}, {0x6A, 0x01}, {0x6D, 0x2C},
    {0x6E, 0x01}, {0x72, 0x01}, {0x73, 0x35}, {0x77, 0x01},

};

constexpr uint8_t register_for_gesture[][2] = {
    {0xEF, 0x00}, {0x41, 0x00}, {0x42, 0x00}, {0xEF, 0x00}, {0x48, 0x3C},
    {0x49, 0x00}, {0x51, 0x10}, {0x83, 0x20}, {0x9f, 0xf9}, {0xEF, 0x01},
    {0x01, 0x1E}, {0x02, 0x0F}, {0x03, 0x10}, {0x04, 0x02}, {0x41, 0x40},
    {0x43, 0x30}, {0x65, 0x96}, {0x66, 0x00}, {0x67, 0x97}, {0x68, 0x01},
    {0x69, 0xCD}, {0x6A, 0x01}, {0x6b, 0xb0}, {0x6c, 0x04}, {0x6D, 0x2C},
    {0x6E, 0x01}, {0x74, 0x00}, {0xEF, 0x00}, {0x41, 0xFF}, {0x42, 0x01},
};

constexpr uint8_t register_for_PS[][2] = {
    {0xEF, 0x00}, {0x41, 0x00}, {0x42, 0x02}, {0x48, 0x20}, {0x49, 0x00},
    {0x51, 0x13}, {0x83, 0x00}, {0x9F, 0xF8}, {0x69, 0x96}, {0x6A, 0x02},
    {0xEF, 0x01}, {0x01, 0x1E}, {0x02, 0x0F}, {0x03, 0x10}, {0x04, 0x02},
    {0x41, 0x50}, {0x43, 0x34}, {0x65, 0xCE}, {0x66, 0x0B}, {0x67, 0xCE},
    {0x68, 0x0B}, {0x69, 0xE9}, {0x6A, 0x05}, {0x6B, 0x50}, {0x6C, 0xC3},
    {0x6D, 0x50}, {0x6E, 0xC3}, {0x74, 0x05},

};

}  // namespace

namespace m5 {
namespace unit {

using namespace m5::utility::mmh3;
using namespace paj7620u2;
using namespace paj7620u2::command;

// class UnitPAJ7620U2
const char UnitPAJ7620U2::name[] = "UnitPAJ7620U2";
const types::uid_t UnitPAJ7620U2::uid{"UnitPAJ7620U2"_mmh3};
const types::uid_t UnitPAJ7620U2::attr{0};

bool UnitPAJ7620U2::begin() {
    uint16_t id{};
    uint8_t ver{};

    m5::utility::delay(1);  // Wait 700us for PAJ7620U2 to stabilize

    if (!get_chip_id(id) || !get_version(ver)) {
        M5_LIB_LOGE("Failed to get id/version %x:%x", id, ver);
        return false;
    }
    if (id != chip_id) {
        M5_LIB_LOGE("Not PAJ7620U2 %x", id);
        return false;
    }

    for (auto&& e : register_for_initialize) {
        if (!writeRegister8(e[0], e[1])) {
            M5_LIB_LOGE("Failed to initilize [%02x]:%x", e[0], e[1]);
            return false;
        }
    }
    return select_bank(0, true);


//    return setMode(_cfg.mode);
}

void UnitPAJ7620U2::update() {
#if 1
    uint16_t detection{};
    if (read_detection(detection) && detection) {
        M5_LIB_LOGE(">>>>>[%x]", detection);

        //    paj7620u2::Gesture _gesture{paj7620u2::Gesture::None};
    }
#endif

#if 0
    if (_periodic) {
        unsigned long at{m5::utility::millis()};
        if (!_latest || at >= _latest + _interval) {
            _interval = 1000;  // 1sec
            _updated  = readMeasurement(_CO2eq, _TVOC);
            if (_updated) {
                _latest = at;
            }
        } else {
            _updated = false;
        }

        // Store baseline values every hour
        if (_interval == 1000 &&
            (!_latestBaseline || at >= _latestBaseline + BASELINE_INTERVAL)) {
            _updatedBaseline = getIaqBaseline(_baselineCO2eq, _baselineTVOC);
            if (_updatedBaseline) {
                _latestBaseline = at;
            }
        } else {
            _updatedBaseline = false;
        }
    }
#endif
}

bool UnitPAJ7620U2::setMode(const paj7620u2::Mode m) {
    size_t len = (m == Mode::Gesture) ? m5::stl::size(register_for_gesture)
                                      : m5::stl::size(register_for_PS);
    auto ptr =
        (m == Mode::Gesture) ? &register_for_gesture[0] : &register_for_PS[0];

    for (size_t i = 0; i < len; ++i) {
        M5_LIB_LOGE("[%02x];%02x", *ptr[0], *ptr[1]);
        if (!writeRegister8(*ptr[0], *ptr[1])) {
            return false;
        }
    }
    _mode = m;
    return true;
}

bool UnitPAJ7620U2::enableSensor(const bool flag) {
    return write_banked_register8(OPERATION_ENABLE, flag ? 1 : 0);
}

bool UnitPAJ7620U2::suspend() {
    return enableSensor(false) && write_banked_register8(SUSPEND_COMMAND, 0);
}

bool UnitPAJ7620U2::resume() {
    // Resume Gesture
    // Write slave ID or I2C read command to process I2C wake-up.
    // It’s recommend to read Reg_0x00. It will return"0x20” when wake-up inish
    uint8_t v{};
    return read_banked_register8(0x00, v) && (v == 0x20) && enableSensor(true);
}

//
bool UnitPAJ7620U2::select_bank(const uint8_t bank, const bool force) {
    if (!force && _current_bank == bank) {
        return true;
    }
    if (writeRegister8(BANK_SEL, bank)) {
        _current_bank = bank;
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::read_banked_register(const uint16_t reg, uint8_t* buf,
                                         const size_t len) {
    return select_bank((reg >> 8) & 1) &&
           readRegister((uint8_t)(reg & 0xFF), buf, len, 0);
}

bool UnitPAJ7620U2::read_banked_register8(const uint16_t reg, uint8_t& value) {
    return select_bank((reg >> 8) & 1) &&
           readRegister8((uint8_t)(reg & 0xFF), value, 0);
}

bool UnitPAJ7620U2::read_banked_register16(const uint16_t reg,
                                           uint16_t& value) {
    return select_bank((reg >> 8) & 1) &&
           readRegister16((uint8_t)(reg & 0xFF), value, 0);
}

bool UnitPAJ7620U2::write_banked_register(const uint16_t reg,
                                          const uint8_t* buf,
                                          const size_t len) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister((uint8_t)(reg & 0xFF), buf, len);
}

bool UnitPAJ7620U2::write_banked_register8(const uint16_t reg,
                                           const uint8_t value) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister8((uint8_t)(reg & 0xFF), value);
}

bool UnitPAJ7620U2::write_banked_register16(const uint16_t reg,
                                            const uint16_t value) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister16((uint8_t)(reg & 0xFF), value);
}

bool UnitPAJ7620U2::get_chip_id(uint16_t& id) {
    return read_banked_register(PART_ID_LOW, (uint8_t*)&id, 2);
}

bool UnitPAJ7620U2::get_version(uint8_t& version) {
    return read_banked_register8(VERSION_ID, version);
}

bool UnitPAJ7620U2::read_detection(uint16_t& detection) {
    if (select_bank(0)) {
        uint8_t high{}, low{};
        if (readRegister8((uint8_t)0x44, high, 0) &&
            readRegister8((uint8_t)0x43, low, 0)) {
            detection = ((uint16_t)high) << 8 | low;
            return true;
        }
    }
    return false;
#if 0
    return read_banked_register(GESTURE_DETECTION_INTERRUPT_FLAG_LOW,
                                (uint8_t*)&detection, 2);
#endif
}

}  // namespace unit
}  // namespace m5
