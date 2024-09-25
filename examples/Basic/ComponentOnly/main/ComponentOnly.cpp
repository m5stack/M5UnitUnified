/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example of using only unit component without UnitUnified manager
  If you use other units, change include files(*1), instances(*2), and get values(*3)
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>  // *1 Include the header of the unit to be used

m5::unit::UnitCO2 unit;  // *2 Instance of the unit

void setup()
{
    M5.begin();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    M5.Display.clear(TFT_DARKGREEN);
    if (!unit.assign(Wire)  // Assign Wire
        || unit.begin()) {  // Begin unit
        M5_LOGE("Failed to assign/begin");
        M5.Display.clear(TFT_RED);
    }
}

void loop()
{
    M5.begin();
    unit.update();  // Explicitly call unit.update() yourself
    if (unit.updated()) {
        // *3 Obtaining unit-specific measurements
        M5_LOGI("CO2:%u Temp:%f Hum:%f", unit.co2(), unit.temperature(), unit.humidity());
    }
}
