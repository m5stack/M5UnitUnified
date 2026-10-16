/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Wiring gate check: must NOT compile

  Without <M5Unified.h> the wiring helpers are not declared (README: "The wiring
  helpers are enabled only when <M5Unified.h> is included before that header").
*/
#include <M5UnitUnified.hpp>
#include <wiring/m5_unit_unified_wiring.hpp>

m5::unit::wiring::I2CPins wiring_gate_without_m5unified()
{
    return m5::unit::wiring::i2cPins();
}
