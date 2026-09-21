/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Arduino library discovery check for M5UnitUnified

  arduino-cli / Arduino IDE add a library to the include path only when a sketch
  (or an already discovered library) includes a header that sits directly in that
  library's src/. A public header that reaches a dependency through a
  subdirectory path (e.g. <m5_utility/...>) is therefore not resolved.

  Keep <M5UnitUnified.h> as the FIRST and ONLY library include here. Including
  M5Unified.h or M5Utility.hpp before it would make the dependencies visible
  and hide exactly the problem this sketch exists to catch.
*/
#include <M5UnitUnified.h>

m5::unit::UnitUnified units;

void setup()
{
}

void loop()
{
    units.update();
}
