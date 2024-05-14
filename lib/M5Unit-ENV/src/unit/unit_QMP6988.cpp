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

namespace {
m5::unit::qmp6988::Accuracy average_to_acc(
    const m5::unit::qmp6988::Average avg) {
    using namespace m5::unit::qmp6988;

    return (avg == Average::Skip)   ? Accuracy::Unknown
           : (avg == Average::Avg1) ? Accuracy::Acc22
           : (avg == Average::Avg2) ? Accuracy::Acc23
                                    : Accuracy::Acc24;
}

inline m5::unit::qmp6988::Accuracy average_to_acc(const uint8_t v) {
    using namespace m5::unit::qmp6988;
    return average_to_acc(static_cast<Average>(v));
}

constexpr uint8_t mode_shift{0};
constexpr uint8_t mode_mask{0x03};
constexpr uint8_t pressure_shift{2};
constexpr uint8_t pressure_mask{0x07 << pressure_shift};
constexpr uint8_t temperature_shift{5};
constexpr uint8_t temperature_mask{0x07 << temperature_shift};

}  // namespace

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

    qmp6988::Average ta;
    qmp6988::Average pa;
    qmp6988::PowerMode mode;
    if (!getMeasurementCondition(ta, pa, mode)) {
        M5_LIB_LOGE("Failed to getMeasurementCondition");
        return false;
    }
    _tempAcc     = average_to_acc(ta);
    _pressureAcc = average_to_acc(pa);
    return true;
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

bool UnitQMP6988::get_measurement_condition(uint8_t& cond) {
    return readRegister8(CONTROL_MEASUREMENT, cond, 0);
}

bool UnitQMP6988::set_measurement_condition(const uint8_t cond) {
    if (writeRegister8(CONTROL_MEASUREMENT, cond)) {
        _tempAcc =
            average_to_acc((cond & temperature_mask) >> temperature_shift);
        _pressureAcc = average_to_acc((cond & pressure_mask) >> pressure_shift);
        return true;
    }
    return false;
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
    if (get_measurement_condition(cond)) {
        ta   = static_cast<qmp6988::Average>((cond & temperature_mask) >>
                                             temperature_shift);
        pa   = static_cast<qmp6988::Average>((cond & pressure_mask) >>
                                             pressure_shift);
        mode = table[cond & mode_mask];
        return true;
    }
    return false;
}

bool UnitQMP6988::setMeasurementCondition(const qmp6988::Average ta,
                                          const qmp6988::Average pa,
                                          const qmp6988::PowerMode mode) {
    uint8_t cond{(uint8_t)((m5::stl::to_underlying(ta) << temperature_shift) |
                           (m5::stl::to_underlying(pa) << pressure_shift) |
                           (m5::stl::to_underlying(mode)))};
    return set_measurement_condition(cond);
}

bool UnitQMP6988::setMeasurementCondition(const qmp6988::Average ta,
                                          const qmp6988::Average pa) {
    uint8_t cond{};
    if (get_measurement_condition(cond)) {
        cond &= mode_mask;
        cond |= (uint8_t)((m5::stl::to_underlying(ta) << temperature_shift) |
                          (m5::stl::to_underlying(pa) << pressure_shift));
        return set_measurement_condition(cond);
    }
    return false;
}

bool UnitQMP6988::setMeasurementCondition(const qmp6988::Average ta) {
    uint8_t cond{};
    if (get_measurement_condition(cond)) {
        cond &= (pressure_mask | mode_mask);
        cond |= (uint8_t)(m5::stl::to_underlying(ta) << temperature_shift);
        return set_measurement_condition(cond);
    }
    return false;
}

bool UnitQMP6988::setPressureOversampling(const qmp6988::Average pa) {
    uint8_t cond{};
    if (get_measurement_condition(cond)) {
        cond &= (temperature_mask | mode_mask);
        cond |= (uint8_t)(m5::stl::to_underlying(pa) << pressure_shift);
        return set_measurement_condition(cond);
    }
    return false;
}

bool UnitQMP6988::setPowerMode(const qmp6988::PowerMode mode) {
    uint8_t cond{};
    if (get_measurement_condition(cond)) {
        cond &= (temperature_mask | pressure_mask);
        cond |= (uint8_t)(m5::stl::to_underlying(mode));
        return set_measurement_condition(cond);
    }
    return false;
}

bool UnitQMP6988::reset() {
    uint8_t v{};
    return writeRegister8(RESET, v);
}

bool UnitQMP6988::softReset() {
    uint8_t v{0xE6};  // When inputting "E6h", a soft-reset will be occurred
    return writeRegister8(RESET, v);
}

bool UnitQMP6988::getStatus(qmp6988::Status& s) {
    return readRegister8(GET_STATUS, s.value, 1);
}

bool UnitQMP6988::getFilterCoeff(qmp6988::Filter& f) {
    constexpr qmp6988::Filter table[] = {
        qmp6988::Filter::Off,
        qmp6988::Filter::Coeff2,
        qmp6988::Filter::Coeff4,
        qmp6988::Filter::Coeff8,
        qmp6988::Filter::Coeff16,
        // 0b101, 0b110, 0b111 are 32
        qmp6988::Filter::Coeff32,
        qmp6988::Filter::Coeff32,
        qmp6988::Filter::Coeff32,
    };

    uint8_t v{};
    if (readRegister8(IIR_FILTER, v, 0)) {
        f = table[v & 0x07];
        return true;
    }
    return false;
}

bool UnitQMP6988::setFilterCoeff(const qmp6988::Filter& f) {
    return writeRegister8(IIR_FILTER, m5::stl::to_underlying(f));
}

}  // namespace unit
}  // namespace m5
