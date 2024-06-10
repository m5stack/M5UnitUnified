/*!
  @file unit_Vmeter.hpp
  @brief Vmeter (ADS1115 + CA-IS3020S) Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_METER_UNIT_V_METER_HPP
#define M5_UNIT_METER_UNIT_V_METER_HPP

#include "unit_ADS1115_with_EEPROM.hpp"

namespace m5 {
namespace unit {

/*!
  @class UnitVmeter
  @brief Voltmeter Unit is a voltage meter that can monitor the voltage in real time
 */
class UnitVmeter : public UnitADS1115WithEEPROM {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x49};
    constexpr static uint8_t DEFAULT_EEPROM_ADDRESS{0x53};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];
    constexpr static float PRESSURE_COEFFICIENT{0.01591895f};
    
    explicit UnitVmeter(const uint8_t addr      = DEFAULT_ADDRESS,
                        const uint8_t epromAddr = DEFAULT_EEPROM_ADDRESS)
        : UnitADS1115WithEEPROM(addr, epromAddr) {
    }

    virtual ~UnitVmeter() {
    }

    //! @brief Resolution of 1 LSB
    inline virtual float resolution() const {
        return coefficient() / PRESSURE_COEFFICIENT;
    }

  protected:
    inline virtual const char* unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }
};
}  // namespace unit
}  // namespace m5
#endif
