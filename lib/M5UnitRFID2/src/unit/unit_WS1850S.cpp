/*!
  @file unit_WS1850S.cpp
  @brief WS1850S Unit for M5UnitUnified

  Functionally compatible with MFRC522

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_WS1850S.hpp"

#include <M5Utility.hpp>
#include <cassert>
#include <array>

namespace {
constexpr uint8_t ws1850s_firmware_version{0x15};

}  // namespace

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

using namespace mfrc522;
using namespace mfrc522::command;

// class UnitWS1850S
const char UnitWS1850S::name[] = "UnitWS1850S";
const types::uid_t UnitWS1850S::uid{"UnitWS1850S"_mmh3};
const types::uid_t UnitWS1850S::attr{0};

bool UnitWS1850S::begin() {
    uint8_t ver{};
    if (!readRegister8(VERSION_REG, ver, 0) ||
        ver != ws1850s_firmware_version) {
        M5_LIB_LOGE("Cannot detect WS1850S %x", ver);
        return false;
    }
    return UnitMFRC522::begin();
}

void UnitWS1850S::update(const bool force) {
    return UnitMFRC522::update(force);
}

}  // namespace unit
}  // namespace m5
