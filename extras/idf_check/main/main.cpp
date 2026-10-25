/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  ESP-IDF build check for M5UnitUnified

  Builds the public headers and the wiring helper as an ESP-IDF component
  consumer. main/CMakeLists.txt declares no esp_* requires, so anything the
  public headers need has to come from M5UnitUnified's own REQUIRES.

  The build log must contain "Support I2C_Class" (the #pragma message in
  adapter_i2c.cpp). If the optional M5Unified requires does not match the
  managed component name, the I2C_Class stub is built instead and add(unit,
  M5.Ex_I2C) / M5.In_I2C fails on the device while the build stays green.
*/
#include <M5UnitUnified.hpp>
#include <M5Unified.h>
#include <wiring/m5_unit_unified_wiring.hpp>

namespace {
m5::unit::UnitUnified units;
}  // namespace

// <M5Unified.h> is included before the wiring header, so the helpers must be declared.
// External linkage keeps this compiled; it is never called.
bool idf_check_wiring(m5::unit::Component& unit)
{
    (void)m5::unit::wiring::i2cPins();
    (void)m5::unit::wiring::addUART(units, unit, 115200, m5::unit::wiring::UartConfig::Default, 1024);
    return m5::unit::wiring::addI2C(units, unit);
}

extern "C" void app_main(void)
{
    M5.begin();
    units.update();
}
