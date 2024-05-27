/*
  Example using M5UnitUnified for UnitHeart(MAX30100)

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/

#define USING_M5HAL  // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_MAX30100.hpp>
#if !defined(USING_M5HAL)
#include <Wire.h>
#endif

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitMAX30100 unitMAX30100;
}  // namespace

void setup() {
    M5.begin();

    if (0) {
        auto cfg         = unitMAX30100.config();
        cfg.samplingRate = m5::unit::max30100::SamplingRate::Sampling1000;
        cfg.pulseWidth   = m5::unit::max30100::LedPulseWidth::PW200;
        unitMAX30100.config(cfg);
    }

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
    if (!Units.add(unitMAX30100, i2c_bus ? i2c_bus.value() : nullptr) ||
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
    if (!Units.add(unitMAX30100, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.clear(TFT_DARKGREEN);
}

void loop() {
    M5.update();
    Units.update();
    if (unitMAX30100.updated()) {
        uint16_t ir{}, red{};
        auto retrived = unitMAX30100.retrived();
        for (uint8_t i = 0; i < retrived; ++i) {
            if (unitMAX30100.getRawData(ir, red, retrived - i - 1)) {
                // For TelePlot(PlatformIO)
                M5_LOGI("\n>IR:%u\n>RED:%u", ir, red);
                // For ArduinoIDE serial plotter
                // M5_LOGI("IR:%u, RED:%u\n", ir, red);
            }
        }
    }
}
