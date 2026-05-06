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

#if defined(ARDUINO)
class TwoWire;
class HardwareSerial;
class SPIClass;
struct SPISettings;
#endif

/*!
  @namespace m5
  @brief Top level namespace of M5Stack
 */
namespace m5 {
class I2C_Class;
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
#if defined(ARDUINO)
    /*!
      @brief Adding unit to be managed (I2C)
      @param u Unit Component
      @param wire TwoWire to be used
      @return True if successful
    */
    bool add(Component& u, TwoWire& wire);
#endif
    /*!
      @brief Adding unit to be managed (GPIO)
      @param u Unit Component
      @param rx_pin Pin number to be used for RX
      @param tx_pin Pin number to be used for TX
      @return True if successful
     */
    bool add(Component& u, const int8_t rx_pin, const int8_t tx_pin);
#if defined(ARDUINO)
    /*!
      @brief Adding unit to be managed (UART)
      @param u Unit Component
      @param serial HardwareSerial to be used
      @return True if successful
    */
    bool add(Component& u, HardwareSerial& serial);
    /*!
      @brief Adding unit to be managed (SPI)
      @param u Unit Component
      @param spi SPI to be used
      @return True if successful
    */
    bool add(Component& u, SPIClass& spi, const SPISettings& settings);
#endif
#if defined(ESP_PLATFORM)
    /*!
      @brief Adding unit to be managed (UART, ESP-IDF native driver)
      @param u Unit Component
      @param uart_num UART port number
      @param baud_rate Baud rate
      @param rx_pin RX pin (-1 to keep)
      @param tx_pin TX pin (-1 to keep)
      @param buf_size RX/TX ring buffer size
      @return True if successful
    */
    bool add(Component& u, uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, int buf_size = 1024);
#endif
    /*!
      @brief Adding unit to be managed (M5HAL)
      @param u Unit Component
      @param bus Bus to be used
      @return True if successful
     */
    bool add(Component& u, m5::hal::bus::Bus* bus);
    /*!
      @brief Adding unit to be managed (I2C_Class)
      @param u Unit Component
      @param i2c I2C_Class to be used (e.g. M5.In_I2C)
      @return True if successful
     */
    bool add(Component& u, m5::I2C_Class& i2c);
#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
    /*!
      @brief Adding unit to be managed (I2C, ESP-IDF native driver)
      @param u Unit Component
      @param bus ESP-IDF I2C master bus handle
      @return True if successful
     */
    bool add(Component& u, i2c_master_bus_handle_t bus);
#endif
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
