/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitRFID2(WS1850S)
*/

// #define USING_M5HAL  // When using M5HAL

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <unit/unit_WS1850S.hpp>
#if !defined(USING_M5HAL)
#include <Wire.h>
#endif

#if defined(ARDUINO)
#include <WString.h>
using string_t = String;
#else
#include <string>
usign string_t = std::string;
#endif

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitWS1850S unit;

string_t toString(const m5::unit::mfrc522::UID& uid) {
    if (uid.size == 0 || uid.size > 10) {
        return "Invalid UID";
    }
    char tmp[32]{};
    uint32_t left{};
    for (uint8_t i = 0; i < uid.size; ++i) {
        left += snprintf(tmp + left, 3, "%02X", uid.uid[i]);
    }
    return string_t(tmp);
}

string_t toString(const m5::unit::mfrc522::PICCType t) {
    static const char* table[] = {
        "Unknown",           "ISO_14443_4",       "ISO_18092",
        "MIFARE_Classic",    "MIFARE_Classic_1K", " MIFARE_Classic_4K",
        "MIFARE_Classic_2K", "MIFARE_UltraLight", "MIFARE_Plus",
        "MIFARE_DESFire",
    };
    auto idx = m5::stl::to_underlying(t);
    return idx < m5::stl::size(table) ? table[idx] : "NotCpmpleted";
}

}  // namespace

void setup() {
    M5.begin();
    M5.Speaker.begin();

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

    lcd.clear(TFT_DARKGREEN);
}

void loop() {
    using namespace m5::unit::mfrc522;

    M5.update();
    Units.update();

    if (M5.BtnC.isPressed()) {
        M5_LOGI("00");
    }

    UID uid{};
    if (unit.piccActivate(uid, false)) {
        M5_LOGI("[%s}:%s", toString(uid.type).c_str(), toString(uid).c_str());
        switch (uid.type) {
            case PICCType::MIFARE_Classic:
            case PICCType::MIFARE_Classic_1K:
            case PICCType::MIFARE_Classic_4K:
            case PICCType::MIFARE_UltraLight:
                M5.Speaker.tone(4000, 20);
                lcd.clear(TFT_BLUE);
                unit.dump(uid);
                break;
            default:
                M5_LOGE("Not support");
                lcd.clear(TFT_PURPLE);
                M5.Speaker.tone(2000, 10);
                M5.Speaker.tone(2000, 10);
                break;
        }
    } else {
        if (M5.BtnA.wasClicked()) {
            lcd.clear(TFT_YELLOW);
            M5.Speaker.tone(4000, 10);
            M5.Speaker.tone(3000, 20);
            M5.Speaker.tone(4000, 10);
            unit.reset();
        }
        lcd.clear(TFT_DARKGREEN);
    }
    // M5_LOGW("---");
}
