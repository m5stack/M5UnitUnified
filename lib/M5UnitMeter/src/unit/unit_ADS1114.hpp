/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_ADS1114.hpp
  @brief ADS1114 Unit for M5UnitUnified
*/
#ifndef M5_UNIT_METER_UNIT_ADS1114_HPP
#define M5_UNIT_METER_UNIT_ADS1114_HPP

#include "unit_ads111x.hpp"

namespace m5 {
namespace unit {
/*!
  @class  UnitADS1114
  @brief ADS1114 unit
 */
class UnitADS1114 : public UnitADS111x {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitADS1114, 0xFF);

   public:
    explicit UnitADS1114(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitADS111x(addr) {
    }
    virtual ~UnitADS1114() {
    }

    ///@name Configration
    ///@{
    /*!  @brief Not support @warning Not support */
    virtual bool setMultiplexer(const ads111x::Mux) override {
        return false;
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
}  // namespace unit
}  // namespace m5
#endif
