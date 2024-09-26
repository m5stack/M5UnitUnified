/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  To update the unit yourself usage example (UnitCO2)
  If you use other units, change include files(*1), instances(*2), and get values(*3)
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedENV.h>  // *1 Include the header of the unit to be used
m5::unit::UnitUnified Units;
m5::unit::UnitCO2 unit;  // *2 Instance of the unit

void update_task(void*)
{
    for (;;) {
        // If exclusive control is required for access to Wire, insert appropriate controls
        unit.update();  // Explicitly call unit.update() yourself
        if (unit.updated()) {
            // *3 Obtaining unit-specific measurements
            M5_LOGI("CO2:%u Temp:%f Hum:%f", unit.co2(), unit.temperature(), unit.humidity());
        }
        m5::utility::delay(1);
    }
}

void setup()
{
    M5.begin();

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    auto ccfg        = unit.component_config();
    ccfg.self_update = true;
    unit.component_config(ccfg);

    M5.Display.clear(TFT_DARKGREEN);
    if (!Units.add(unit, Wire)  // Add unit to UnitUnified manager
        || !Units.begin()) {    // Begin each unit
        M5_LOGE("Failed to add/begin");
        M5.Display.clear(TFT_RED);
        return;
    }
    xTaskCreateUniversal(update_task, "update_task", 8192, nullptr, 1, nullptr,
#if defined(CONFIG_IDF_TARGET_ESP32C6)
                         PRO_CPU_NUM);
#else
                         APP_CPU_NUM);
#endif
}

void loop()
{
    M5.begin();
    Units.update();  // unit.update() is not called within this function
    M5_LOGI("loop");
    m5::utility::delay(1000);
}
