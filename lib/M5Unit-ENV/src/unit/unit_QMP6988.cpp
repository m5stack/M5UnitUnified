/*!
  @file unit_QMP6988.cpp
  @brief QMP6988 Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#include "unit_QMP6988.hpp"
#include <M5Utility.hpp>

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

using namespace qmp6988::command;

const char UnitQMP6988::name[] = "UnitQMP6988";
const types::uid_t UnitQMP6988::uid{"UnitQMP6988"_mmh3};
const types::uid_t UnitQMP6988::attr{0};

bool UnitQMP6988::begin() {
#if 0
    auto r = stopPeriodicMeasurement();
    if (!r) {
        M5_LIB_LOGD("Failed to stop");
        return false;
    }
    return startPeriodicMeasurement();
#endif

    return reset();
}

void UnitQMP6988::update() {
#if 0
    if (inPeriodic()) {
        unsigned long at{m5::utility::millis()};
        if (!_latest || at >= _latest + _interval) {
            _updated = readMeasurement();
            if (_updated) {
                _latest = at;
            }
        } else {
            _updated = false;
        }
    }
#endif
}

bool UnitQMP6988::getMeasurementCondition(qmp6988::Average& ta,
                                          qmp6988::Average& pa,
                                          qmp6988::PowerMode& mode) {
    // 0b01 and 0b10 are Force
    constexpr qmp6988::PowerMode table[] = {
        qmp6988::PowerMode::Sleep,
        qmp6988::PowerMode::Force,
        qmp6988::PowerMode::Force,
        qmp6988::PowerMode::Normal,
    };

    uint8_t cond{};
    if (readRegister(CONTROL_MEASUREMENT, cond, 0)) {
        M5_LIB_LOGI("[%x]", cond);

        ta   = static_cast<qmp6988::Average>((cond >> 5U) & 0x07);
        pa   = static_cast<qmp6988::Average>((cond >> 2U) & 0x07);
        mode = table[cond & 0x03];
        return true;
    }
    return false;
}
bool UnitQMP6988::setMeasurementCondition(const qmp6988::Average ta,
                                          const qmp6988::Average pa,
                                          const qmp6988::PowerMode mode) {
    m5::types::big_uint16_t cmd(CONTROL_MEASUREMENT,
                                (uint8_t)((m5::stl::to_underlying(ta) << 5U) |
                                          (m5::stl::to_underlying(pa) << 2U) |
                                          (m5::stl::to_underlying(mode))));

    M5_LIB_LOGI("%x", cmd.u16);

    return (writeWithTransaction(cmd.data(), cmd.size()) ==
            m5::hal::error::error_t::OK);
}

bool UnitQMP6988::setPowerMode(const qmp6988::PowerMode mode) {
    qmp6988::Average t;
    qmp6988::Average p;
    qmp6988::PowerMode m;
    if (getMeasurementCondition(t, p, m)) {
        return setMeasurementCondition(t, p, mode);
    }
    return false;
}

bool UnitQMP6988::setPresureOversampling(const qmp6988::Average pa) {
    qmp6988::Average t;
    qmp6988::Average p;
    qmp6988::PowerMode m;
    if (getMeasurementCondition(t, p, m)) {
        return setMeasurementCondition(t, pa, m);
    }
    return false;
}

bool UnitQMP6988::setTemperatureOversampling(const qmp6988::Average ta) {
    qmp6988::Average t;
    qmp6988::Average p;
    qmp6988::PowerMode m;
    if (getMeasurementCondition(t, p, m)) {
        return setMeasurementCondition(ta, p, m);
    }
    return false;
}

bool UnitQMP6988::reset() {
    uint8_t buf[] = {RESET, 00};
    return (writeWithTransaction(buf, 2) == m5::hal::error::error_t::OK);
}

bool UnitQMP6988::getStatus(qmp6988::Status& s) {
    return false;
}

}  // namespace unit
}  // namespace m5
