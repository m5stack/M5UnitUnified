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
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    constexpr static uint8_t DEFAULT_EEPROM_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

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
    inline virtual const char* unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }

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
