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

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitWS1850S unit;

void printUID(const m5::unit::mfrc522::UID& uid) {
    char ids[2 * 10 + 1]{};
    if (uid.size == 0 || uid.size > 10) {
        M5_LOGE("Invalid UID:%u", uid.size);
        return;
    }

    uint8_t left{};
    for (uint8_t i = 0; i < uid.size; ++i) {
        left += snprintf(ids + left, 3, "%02X", uid.uid[i]);
    }
    M5_LOGI("Type:%u\nUID:%s", uid.sak, ids);
}

}  // namespace

void setup() {
    delay(3000);

    M5.begin();

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

void write_test(m5::unit::mfrc522::UID& uid);

void loop() {
    M5.update();
    Units.update();

    m5::unit::mfrc522::UID uid{};
    //    uid = { 4, {0x2A, 0x26, 0xFB, 0x70}, 8 };
    if (unit.piccActivate(uid, false)) {
        printUID(uid);
        write_test(uid);
        // unit.dump(uid);
    }
}

void write_test(m5::unit::mfrc522::UID& uid) {
    uint8_t sector   = 1;
    uint8_t block    = 4;
    uint8_t data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    m5::unit::UnitMFRC522::result_t result{};
    switch (uid.type) {
        case m5::unit::mfrc522::PICCType::MIFARE_Classic:
        case m5::unit::mfrc522::PICCType::MIFARE_Classic_1K:
        case m5::unit::mfrc522::PICCType::MIFARE_Classic_4K:
            M5_LOGI("Try write to classic");
            result =
                // Authenticate A
                unit.piccAuthenticateWithKeyA(
                        uid, m5::unit::UnitMFRC522::DEFAULT_CLASSIC_KEY_A,
                        block)
                    .and_then([&uid, &block]() {
                        // Authenticate B
                        return unit.piccAuthenticateWithKeyB(
                            uid, m5::unit::UnitMFRC522::DEFAULT_CLASSIC_KEY_A,
                            block);
                    })
                    .and_then([&block, &data]() {
                        // Write
                        return unit.mifareWrite(block, data, 16);
                    });
            break;
        case m5::unit::mfrc522::PICCType::MIFARE_UltraLight:
            M5_LOGI("Try write to ultralight");
            result = unit.mifareUltralightWrite(4, (const uint8_t*)"DATA", 4);
            break;
        default:
            return;
    }
    if (result) {
        M5_LOGW("Wrote");
        unit.dump(uid);
    } else {
        M5_LOGE("ERR:%u", result.error());
        unit.piccHLTA();
        unit.stopCrypto1();
    }
}
