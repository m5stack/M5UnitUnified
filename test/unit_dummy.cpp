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
#include <m5_unit_component/types.hpp>

using namespace m5::utility::mmh3;
using namespace m5::unit::types::attribute;

namespace m5 {
namespace unit {

// UnitDummy: I2C accessible
const char UnitDummy::name[] = "UnitDummy";
const types::uid_t UnitDummy::uid{"UnitDummy"_mmh3};
const types::attr_t UnitDummy::attr{AccessI2C};

// UnitDummyGPIO: GPIO accessible
const char UnitDummyGPIO::name[] = "UnitDummyGPIO";
const types::uid_t UnitDummyGPIO::uid{"UnitDummyGPIO"_mmh3};
const types::attr_t UnitDummyGPIO::attr{AccessGPIO};

// UnitDummyUART: UART accessible
const char UnitDummyUART::name[] = "UnitDummyUART";
const types::uid_t UnitDummyUART::uid{"UnitDummyUART"_mmh3};
const types::attr_t UnitDummyUART::attr{AccessUART};

// UnitDummySPI: SPI accessible
const char UnitDummySPI::name[] = "UnitDummySPI";
const types::uid_t UnitDummySPI::uid{"UnitDummySPI"_mmh3};
const types::attr_t UnitDummySPI::attr{AccessSPI};

// UnitDummyI2CSPI: I2C + SPI accessible
const char UnitDummyI2CSPI::name[] = "UnitDummyI2CSPI";
const types::uid_t UnitDummyI2CSPI::uid{"UnitDummyI2CSPI"_mmh3};
const types::attr_t UnitDummyI2CSPI::attr{AccessI2C | AccessSPI};

// UnitDummyAll: All accessible
const char UnitDummyAll::name[] = "UnitDummyAll";
const types::uid_t UnitDummyAll::uid{"UnitDummyAll"_mmh3};
const types::attr_t UnitDummyAll::attr{AccessI2C | AccessGPIO | AccessUART | AccessSPI};

// UnitDummyNone: No access
const char UnitDummyNone::name[] = "UnitDummyNone";
const types::uid_t UnitDummyNone::uid{"UnitDummyNone"_mmh3};
const types::attr_t UnitDummyNone::attr{0};

}  // namespace unit
}  // namespace m5
