/*
  Example using M5UnitUnified for PAJ7620U2

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

// #define USING_PAHUB (2)  // Connection channel number for use via PaHub.
// #define USING_M5HAL      // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_PAJ7620U2.hpp>
#if defined(USING_PAHUB)
#include <unit/unit_PaHub.hpp>
#endif

namespace {
auto& lcd = M5.Display;

constexpr uint32_t wire_freq = 400000U;

m5::unit::UnitUnified Units;
m5::unit::UnitPAJ7620U2 unit;
#if defined(USING_PAHUB)
m5::unit::UnitPaHub unitPaHub;
#endif

constexpr const char* gstr[] = {
    "None",          "Left ",     "Right",
    "Down",          "Up",        "Forward",
    "Backward",      "Clockwise", "CounterClockwise",
    "Wave",          "Approach",  "HasObject",
    "WakeupTrigger", "Confirm",   "Abort",
    "Reserve",       "NoObject",
};

const char* gesture_to_string(const m5::unit::paj7620u2::Gesture g) {
    unsigned int ui{m5::stl::to_underlying(g)};
    ui = (ui == 0) ? 0 : __builtin_ctz(ui) + 1;

    return ui < m5::stl::size(gstr) ? gstr[ui] : "ERROR";
}

using namespace m5::unit::paj7620u2;
DetectionMode& operator++(DetectionMode& m) {
    uint8_t v = m5::stl::to_underlying(m) + 1;
    if (v > m5::stl::to_underlying(DetectionMode::Cursor)) {
        v = 0;
    }
    m = static_cast<DetectionMode>(v);
    return m;
}
DetectionMode& operator--(DetectionMode& m) {
    uint8_t v = m5::stl::to_underlying(m) - 1;
    if (v > m5::stl::to_underlying(DetectionMode::Cursor)) {
        v = m5::stl::to_underlying(DetectionMode::Cursor);
    }
    m = static_cast<DetectionMode>(v);
    return m;
}
DetectionMode detection{DetectionMode::Gesture};

}  // namespace

void setup() {
    m5::utility::delay(2000);

    M5.begin();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);

#if defined(USING_PAHUB)
#pragma message "Using via PaHub"
    // Using via PaHub
#if defined(USING_M5HAL)
#pragma message "Using M5HAL"
    // Using M5HAL
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);

    if (!unitPaHub.add(unit, USING_PAHUB) ||
        !Units.add(unitPaHub, i2c_bus ? i2c_bus.value() : nullptr) ||
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#else
    // Using TwoWire
#pragma message "Using Wire"
    Wire.begin(pin_num_sda, pin_num_scl, wire_freq);
    if (!unitPaHub.add(unit, USING_PAHUB) || !Units.add(unitPaHub, Wire) ||
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif

#else
    // Direct connection
#pragma message "Direct connection"
#if defined(USING_M5HAL)
#pragma message "Using M5HAL"
    // Using M5HAL
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
    if (!Units.add(unit, i2c_bus ? i2c_bus.value() : nullptr) ||
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#else
#pragma message "Using Wire"
    // Using TwoWire
    Wire.begin(pin_num_sda, pin_num_scl, wire_freq);
    M5_LOGW("Wire begin");

    if (!Units.add(unit, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.clear(TFT_DARKGREEN);
}

void loop() {
    M5.update();
    Units.update();
    switch (unit.detectionMode()) {
        case m5::unit::paj7620u2::DetectionMode::Gesture: {
            static uint8_t noobj{};

            if (unit.updated()) {
                uint8_t nomot{};
                uint16_t size{};
                uint16_t x{}, y{};
                unit.readNoMotionCount(nomot);
                unit.readObjectSize(size);
                unit.readObjectCenter(x,y);

                M5_LOGI("%lu:gesture:%s noobject:%u nomotion:%u size:%u (%u,%u)",
                        unit.updatedMillis(), gesture_to_string(unit.gesture()),
                        noobj, nomot, size, x, y);
            }
            unit.readNoObjectCount(noobj);
        } break;
        case m5::unit::paj7620u2::DetectionMode::Proximity: {
            if (unit.updated()) {
                M5_LOGI("%lu:%s brightness:%u approch:%u", unit.updatedMillis(),
                        gesture_to_string(unit.gesture()), unit.brightness(),
                        unit.approach());
            }
        } break;
        case m5::unit::paj7620u2::DetectionMode::Cursor: {
            if (unit.updated()) {
                M5_LOGI("%lu:%s cursor:%u,%u", unit.updatedMillis(),
                        gesture_to_string(unit.gesture()), unit.cursorX(),
                        unit.cursorY());
            }
        } break;
        default:
            break;
    }
    if (M5.BtnA.wasClicked()) {
        auto prev = detection;
        ++detection;
        if (unit.setDetectionMode(detection)) {
            M5_LOGI(">>> setNMode %x", detection);
            switch (unit.detectionMode()) {
                case m5::unit::paj7620u2::DetectionMode::Gesture:
                    unit.setMode(Mode::Gaming);
                    break;
                case m5::unit::paj7620u2::DetectionMode::Proximity:
                    unit.setApprochThreshold(2, 1);
                    break;
                case m5::unit::paj7620u2::DetectionMode::Cursor:
                    break;
                default:
                    break;
            }
        } else {
            M5_LOGE("Failed to setMode %x", detection);
            detection = prev;
        }
    }
}
