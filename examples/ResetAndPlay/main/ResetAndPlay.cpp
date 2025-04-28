/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example: Reset and play using TOF units
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedTOF.h>
#include <M5Utility.h>

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;

m5::unit::UnitToF unitToF;
m5::unit::UnitToF4M unitToF4M;

m5::unit::UnitToF* pUnitToF;
m5::unit::UnitToF4M* pUnitToF4M;

int16_t lastRange{};
}  // namespace

void setup()
{
    M5.begin();

    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    // Add all units
    if (!Units.add(unitToF, Wire) || !Units.add(unitToF4M, Wire) || !Units.begin(true /* ignore error on begin */)) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) m5::utility::delay(1000);
    }
    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    // Reset and Play!
    pUnitToF   = Units.get<m5::unit::UnitToF>();
    pUnitToF4M = Units.get<m5::unit::UnitToF4M>();

    if (pUnitToF) {
        M5.Log.printf("Found UnitTOF\n");
        lcd.setTextColor(TFT_ORANGE);
    } else if (pUnitToF4M) {
        M5.Log.printf("Found UnitTOF4M\n");
        lcd.setTextColor(TFT_CYAN);
    } else {
        M5_LOGE("Not exists unit");
        lcd.fillScreen(TFT_RED);
        while (true) m5::utility::delay(1000);
    }

    lcd.setFont(&fonts::Orbitron_Light_32);
    lcd.setTextDatum(middle_center);
    float scale = lcd.width() / (32 * 4.0f);
    lcd.setTextSize(scale, scale);

    lcd.startWrite();
    lcd.clear();
}

void loop()
{
    M5.update();
    Units.update();

    // For UnitToF
    if (pUnitToF && pUnitToF->updated()) {
        auto range = pUnitToF->range();
        if (range >= 0 && range != lastRange) {
            lcd.fillRect(0, lcd.height() / 2 - lcd.fontHeight() / 2, lcd.width(), lcd.fontHeight(), TFT_BLACK);
            lcd.drawString(m5::utility::formatString("%d", range).c_str(), lcd.width() / 2, lcd.height() / 2);
            lastRange = range;
        }
    }
    // For UnitToF4M
    if (pUnitToF4M && pUnitToF4M->updated()) {
        auto range = pUnitToF4M->range();
        if (range >= 0 && range != lastRange) {
            lcd.fillRect(0, lcd.height() / 2 - lcd.fontHeight() / 2, lcd.width(), lcd.fontHeight(), TFT_BLACK);
            lcd.drawString(m5::utility::formatString("%d", range).c_str(), lcd.width() / 2, lcd.height() / 2);
            lastRange = range;
        }
    }
}
