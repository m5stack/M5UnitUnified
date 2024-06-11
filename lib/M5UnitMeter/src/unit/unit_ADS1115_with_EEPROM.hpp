/*!
  @file unit_ADS1115_with_EEPROM.hpp
  @brief Base class for Ameter and Vmeter

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_METER_UNIT_ADS1115_WITH_EEPROM_HPP
#define M5_UNIT_METER_UNIT_ADS1115_WITH_EEPROM_HPP

#include "unit_ADS111x.hpp"

namespace m5 {
namespace unit {

/*!
  @class UnitADS1115WithEEPROM
  @brief EEPROM holds calibration data
  @warning EEPROM  has built-in calibration parameters when leaving the factory.
  Please do not write to the EEPROM, otherwise the calibration data will be
  overwritten and the measurement results will be inaccurate.
 */
class UnitADS1115WithEEPROM : public UnitADS1115 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitADS1115WithEEPROM, 0xFF);

   public:
    constexpr static uint8_t DEFAULT_EEPROM_ADDRESS{0xFF};

    explicit UnitADS1115WithEEPROM(
        const uint8_t addr      = DEFAULT_ADDRESS,
        const uint8_t epromAddr = DEFAULT_EEPROM_ADDRESS)
        : UnitADS1115(addr), _eepromAddr(epromAddr) {
    }
    virtual ~UnitADS1115WithEEPROM() {
    }

    virtual bool assign(m5::hal::bus::Bus* bus) override;
    virtual bool assign(TwoWire& wire) override;
    virtual bool begin() override;

    virtual bool setGain(const ads111x::Gain gain) override;

    //! @brief calibration factor
    float calibrationFactor() const {
        return _calibrationFactor;
    }

   protected:
    bool read_calibration(const ads111x::Gain gain, int16_t& hope,
                          int16_t& actual);
    void apply_calibration(const ads111x::Gain gain);

   protected:
    uint8_t _eepromAddr{};
    std::unique_ptr<m5::unit::Adapter> _adapterEEPROM{};
    ///@cond
    struct Calibration {
        int16_t hope{1};
        int16_t actual{1};
    };
    ///@endcond
    Calibration _calibration[8 /*Gain*/]{};
    float _calibrationFactor{1.0f};
};

}  // namespace unit
}  // namespace m5
#endif
