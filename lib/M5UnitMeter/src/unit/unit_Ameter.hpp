/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_Ameter.hpp
  @brief Ameter (ADS1115 + CA-IS3020S) Unit for M5UnitUnified
*/
#ifndef M5_UNIT_METER_UNIT_A_METER_HPP
#define M5_UNIT_METER_UNIT_A_METER_HPP

#include "unit_ADS1115.hpp"
#include <limits>  // NaN

namespace m5 {
namespace unit {

/*!
  @class UnitAmeter
  @brief Ameter Unit is a current meter that can monitor the current in real
  time
*/
class UnitAmeter : public UnitADS1115WithEEPROM {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitAmeter, 0x48);

   public:
    constexpr static uint8_t DEFAULT_EEPROM_ADDRESS{0x51};
    constexpr static float PRESSURE_COEFFICIENT{0.05f};

    explicit UnitAmeter(const uint8_t addr      = DEFAULT_ADDRESS,
                        const uint8_t epromAddr = DEFAULT_EEPROM_ADDRESS)
        : UnitADS1115WithEEPROM(addr, epromAddr) {
    }
    virtual ~UnitAmeter() {
    }

    //! @brief Resolution of 1 LSB
    inline float resolution() const {
        return coefficient() / PRESSURE_COEFFICIENT;
    }
    //! @brief Gets the correction value
    inline float correction() const {
        return _correction;
    }
    //! @brief Oldest current
    inline float current() const {
        return !empty() ? correction() * std::abs(adc())
                        : std::numeric_limits<float>::quiet_NaN();
    }

   protected:
    virtual void apply_coefficient(const ads111x::Gain gain) override;

   protected:
    float _correction{1.0f};
};
}  // namespace unit
}  // namespace m5
#endif
