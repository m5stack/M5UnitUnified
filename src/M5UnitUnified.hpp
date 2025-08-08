/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnified.hpp
  @brief Main header of M5UnitUnified

  @mainpage M5UnitUnified
  M5UnitUnified  is a library for unified handling of various M5 unit products.
  - Unified APIs
  - Unified Connections
  - Unified Licensing
*/
#ifndef M5_UNIT_UNIFIED_HPP
#define M5_UNIT_UNIFIED_HPP

#include "M5UnitComponent.hpp"
#include <M5HAL.hpp>
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
#else
#include <driver/rmt.h>
#endif
#include <vector>
#include <string>

class TwoWire;
class HardwareSerial;

/*!
  @namespace m5
  @brief Top level namespace of M5stack
 */
namespace m5 {
/*!
  @namespace unit
  @brief Unit-related namespace
 */
namespace unit {
class Component;

/*!
  @class m5::unit::UnitUnified
  @brief For managing and leading units
 */
class UnitUnified {
public:
    using container_type = std::vector<Component*>;

    ///@warning COPY PROHIBITED
    ///@name Constructor
    ///@{
    UnitUnified()                       = default;
    UnitUnified(const UnitUnified&)     = delete;
    UnitUnified(UnitUnified&&) noexcept = default;
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Assignment
    ///@{
    UnitUnified& operator=(const UnitUnified&) = delete;

    UnitUnified& operator=(UnitUnified&&) noexcept = default;
    ///@}

    ///@name Add unit
    ///@{
    /*!
      @brief Adding unit to be managed (I2C)
      @param u Unit Component
      @param wire TwoWire to be used
      @return True if successful
    */
    bool add(Component& u, TwoWire& wire);
    /*!
      @brief Adding unit to be managed (GPIO)
      @param u Unit Component
      @param rx_pin Pin number to be used for RX
      @param tx_pin Pin number to be used for TX
      @return True if successful
     */
    bool add(Component& u, const int8_t rx_pin, const int8_t tx_pin);
    /*!
      @brief Adding unit to be managed (UART)
      @param u Unit Component
      @param serial HardwareSerial to be used
      @return True if successful
    */
    bool add(Component& u, HardwareSerial& wire);
    /*!
      @brief Adding unit to be managed (M5HAL)
      @param u Unit Component
      @param bus Bus to be used
      @return True if successful
     */
    bool add(Component& u, m5::hal::bus::Bus* bus);
    ///@}

    //! @brief Begin of all units under management
    bool begin();
    //! @brief Update of all units under management
    void update(const bool force = false);

    //! @brief Output information for debug
    std::string debugInfo() const;

protected:
    bool add_children(Component& u);
    std::string make_unit_info(const Component* u, const uint8_t indent = 0) const;

protected:
    container_type _units{};

private:
    static uint32_t _registerCount;
};

}  // namespace unit
}  // namespace m5

#endif
