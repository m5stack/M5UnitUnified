/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for M5UnitComponent
*/
#include "unit_dummy.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;

const char UnitDummy::name[] = "UnitDummy";
const m5::unit::types::uid_t UnitDummy::uid{"UnitDummy"_mmh3};
const m5::unit::types::attr_t UnitDummy::attr{0};
