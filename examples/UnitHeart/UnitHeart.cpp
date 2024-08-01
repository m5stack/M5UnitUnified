/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitHeart(MAX30100)
*/

// #define USING_M5HAL  // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_MAX30100.hpp>
#include <utility/heart_rate.hpp>
#if !defined(USING_M5HAL)
#include <Wire.h>
#endif

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitMAX30100 unitMAX30100;
m5::max30100::HeartRate heartRate(100);

uint32_t getSamplingRate(const m5::unit::max30100::Sampling rate) {
    static constexpr uint32_t table[] = {50,  100, 167, 200,
                                         400, 600, 800, 1000};
    return table[m5::stl::to_underlying(rate)];
}

}  // namespace

void setup() {
    M5.begin();

    // Another settings
    if (0) {
        auto cfg         = unitMAX30100.config();
        cfg.samplingRate = m5::unit::max30100::Sampling::Rate400;
        cfg.pulseWidth   = m5::unit::max30100::LedPulseWidth::PW400;
        cfg.irCurrent    = m5::unit::max30100::CurrentControl::mA7_6;
        cfg.redCurrent   = m5::unit::max30100::CurrentControl::mA7_6;
        heartRate.setSamplingRate(getSamplingRate(cfg.samplingRate));
        heartRate.setThreshold(25.0f);  // depends on ir/redCurrent
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
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);
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
        while (unitMAX30100.available()) {
            // M5_LOGI("\n>IR:%u\n>RED:%u", unitMAX30100.ir(),
            // unitMAX30100.red());
            bool beat = heartRate.push_back((float)unitMAX30100.ir(),
                                            (float)unitMAX30100.red());
            if (beat) {
                M5_LOGI("Beat!");
            }

            unitMAX30100.discard();
        }
        auto bpm = heartRate.calculate();
        M5_LOGW("\n>HRR:%f\n>SpO2:%f", bpm, heartRate.SpO2());
    }

    // buffer clear and measure tempeature
    if (M5.BtnA.wasClicked()) {
        heartRate.clear();
        m5::unit::max30100::TemperatureData td{};
        if (unitMAX30100.measureTemperatureSingleshot(td)) {
            M5_LOGI("\n>Temp:%f", td.celsius());
        }
    }
}
