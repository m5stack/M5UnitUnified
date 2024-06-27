/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example: UnitAmeter
*/

// #define USING_M5HAL  // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_Ameter.hpp>
#if !defined(USING_M5HAL)
#include <Wire.h>
#endif

using namespace m5::unit::ads111x;

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitAmeter unit;

float correction{1.0f};
Rate rate{Rate::SPS_32};
constexpr uint16_t sps_table[] = {8, 16, 32, 64, 128, 250, 475, 860};

}  // namespace

void setup() {
    M5.begin();

    m5::utility::delay(3000);

    // Settings
    auto cfg = unit.config();
    cfg.gain = Gain::PGA_1024;
    cfg.rate = rate;
    unit.config(cfg);

    //
    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);

#if defined(USING_M5HAL)
#pragma message "Using M5HAL"
    // Using M5HAL
    m5::hal::bus::I2CBusConfig i2c_cfg;
    i2c_cfg.pin_sda = m5::hal::gpio::getPin(pin_num_sda);
    i2c_cfg.pin_scl = m5::hal::gpio::getPin(pin_num_scl);
    auto i2c_bus    = m5::hal::bus::i2c::getBus(i2c_cfg);
    M5_LOGI("Bus:%d", i2c_bus.has_value());
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
    Wire.begin(pin_num_sda, pin_num_scl, 100000U);
    if (!Units.add(unit, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    correction = unit.resolution() * unit.calibrationFactor();

    M5_LOGI(">>RES:%f COEE:%f CF:%f periodic :%d", unit.resolution(),
            unit.coefficient(), unit.calibrationFactor(), unit.inPeriodic());

    lcd.clear(TFT_DARKGREEN);
}

void loop() {
    M5.update();
    Units.update();
    if (unit.updated()) {
        int16_t raw{};
        raw = unit.latestData();
        // Line graphs can be viewed on a plotter
        // ArduinoIDE, PlatformIO with Teleolot
        M5_LOGI("\n>Current:%f", std::fabs(raw) * correction);
    }

    if (M5.BtnA.wasClicked()) {
        rate = static_cast<Rate>((m5::stl::to_underlying(rate) + 1) & 0x07);
        M5_LOGI("Change to %u sps", sps_table[m5::stl::to_underlying(rate)]);
        if (unit.setRate(rate)) {
            correction = unit.resolution() * unit.calibrationFactor();
        }
    }
}
