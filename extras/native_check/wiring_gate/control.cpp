/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Control for the wiring gate checks: must compile

  Uses only what the wiring header declares without M5Unified, so that the
  failing checks fail because of the gate and not because of the setup.
*/
#include <M5UnitUnified.hpp>
#include <wiring/m5_unit_unified_wiring.hpp>

m5::unit::wiring::I2CPins wiring_gate_control()
{
    return m5::unit::wiring::I2CPins{};
}
