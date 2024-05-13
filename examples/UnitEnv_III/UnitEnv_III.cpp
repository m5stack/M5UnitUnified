/*
  Example using M5UnitUnified for UnitENVIII

  copyright M5Stack. All rights reserved.
  Licensed under the MIT license. See LICENSE file in the project root for full
  license information.
*/

#define USING_PAHUB (4)  // Connection channel number for use via PaHub.
// #define USING_M5HAL      // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_SHT3x.hpp>
#if defined(USING_PAHUB)
#include <unit/unit_PaHub.hpp>
#endif

namespace {
auto& lcd = M5.Display;

m5::unit::UnitUnified Units;
m5::unit::UnitSHT30 unitSHT30;
#if defined(USING_PAHUB)
m5::unit::UnitPaHub unitPaHub;
#endif

}  // namespace

void setup() {
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

    if (!unitPaHub.add(unitSHT30, USING_PAHUB) ||
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
    Wire.begin(pin_num_sda, pin_num_scl, 400000U);
    if (!unitPaHub.add(unitSHT30, USING_PAHUB) || !Units.add(unitPaHub, Wire) ||
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
    if (!Units.add(unitSHT30, i2c_bus ? i2c_bus.value() : nullptr) ||
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
    if (!Units.add(unitSHT30, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif
#endif

#if 0    
    // This is the imagined code that will happen after M5HAL is serviced and
    // M5Unified is modified accordingly.
    if (!M5.Units.add(unitSHT30, M5.PortA) || !M5.Units.begin()) {
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

#if 0    
    // This is the imagined code that will happen after M5HAL is serviced and
    // M5Unified is modified accordingly.
    M5.update(); // Call M5.Units.update() in it.
#endif

    if (unitSHT30.updated()) {
        M5_LOGI("Temperature:%2.2f Humidity:%2.2f", unitSHT30.temperature(),
                unitSHT30.humidity());
    }
}
