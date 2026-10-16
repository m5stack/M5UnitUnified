/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Wiring gate check: must NOT compile

  <M5Unified.h> included AFTER the wiring header does not enable the helpers.
  M5Unified cannot be built for the host, so its include guard stands in for it.
*/
#include <M5UnitUnified.hpp>
#include <wiring/m5_unit_unified_wiring.hpp>
#define __M5UNIFIED_HPP__  // as if <M5Unified.h> were included here

m5::unit::wiring::I2CPins wiring_gate_m5unified_after()
{
    return m5::unit::wiring::i2cPins();
}
