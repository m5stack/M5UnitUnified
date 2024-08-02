/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_ADS1115.hpp
  @brief ADS1115 Unit for M5UnitUnified
*/
#ifndef M5_UNIT_METER_UNIT_ADS1115_HPP
#define M5_UNIT_METER_UNIT_ADS1115_HPP

#include "unit_ads111x.hpp"

namespace m5 {
namespace unit {

/*!
  @class  UnitADS1115
  @brief ADS1115 unit
 */
class UnitADS1115 : public UnitADS111x {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitADS1115, 0x00);

   public:
    explicit UnitADS1115(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitADS111x(addr) {
    }
    virtual ~UnitADS1115() {
    }

    ///@name Configration
    ///@{
    /*! @brief Set the input multiplexer */
    virtual bool setMultiplexer(const ads111x::Mux mux) override {
        return set_multiplexer(mux);
    }
    //! @brief Set the programmable gain amplifier
    virtual bool setGain(const ads111x::Gain gain) override {
        return set_gain(gain);
    }
    //! @brief Set the comparator mode
    virtual bool setComparatorMode(const bool b) override {
        return set_comparator_mode(b);
    }
    //! @brief Set the comparator polarity
    virtual bool setComparatorPolarity(const bool b) override {
        return set_comparator_polarity(b);
    }
    //! @brief Set the latching comparator
    virtual bool setLatchingComparator(const bool b) override {
        return set_latching_comparator(b);
    }
    //! @brief Set the comparator queue
    virtual bool setComparatorQueue(const ads111x::ComparatorQueue c) override {
        return set_comparator_queue(c);
    }
    ///@}

   protected:
    virtual bool on_begin() override;
};

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

    virtual bool setGain(const ads111x::Gain gain) override;

    //! @brief calibration factor
    float calibrationFactor() const {
        return _calibrationFactor;
    }

   protected:
    virtual bool on_begin() override;
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
