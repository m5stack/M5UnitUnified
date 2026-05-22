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

    ///@name Add unit(I2C)
    ///@{
#if defined(ARDUINO)
    /*!
      @brief Add unit to be managed (I2C via TwoWire)
      @param u Unit Component
      @param wire TwoWire to be used
      @return True if successful
    */
    bool add(Component& u, TwoWire& wire);
#endif
#if defined(ESP_PLATFORM) && __has_include(<driver/i2c_master.h>)
    /*!
      @brief Adding unit to be managed (I2C, ESP-IDF native driver)
      @param u Unit Component
      @param bus ESP-IDF I2C master bus handle
      @return True if successful
     */
    bool add(Component& u, i2c_master_bus_handle_t bus);
#elif defined(ESP_PLATFORM)
    /*!
      @brief Adding unit to be managed (I2C, ESP-IDF legacy driver)
      @param u Unit Component
      @param port I2C port (driver must be installed beforehand via i2c_param_config / i2c_driver_install)
      @param sda SDA GPIO
      @param scl SCL GPIO
      @return True if successful
     */
    bool add(Component& u, const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl);
#endif
    /*!
      @brief Add unit to be managed (I2C via I2C_Class)
      @param u Unit Component
      @param i2c I2C_Class to be used (e.g. M5.In_I2C)
      @return True if successful
     */
    bool add(Component& u, m5::I2C_Class& i2c);
    ///@}

    ///@name Add unit(GPIO)
    ///@{
    /*!
      @brief Add unit to be managed (GPIO)
      @param u Unit Component
      @param rx_pin Pin number to be used for RX
      @param tx_pin Pin number to be used for TX
      @return True if successful
     */
    bool add(Component& u, const int8_t rx_pin, const int8_t tx_pin);
    ///@}

    ///@name Add unit(UART)
    ///@{
#if defined(ARDUINO)
    /*!
      @brief Add unit to be managed (UART)
      @param u Unit Component
      @param serial HardwareSerial to be used
      @return True if successful
    */
    bool add(Component& u, HardwareSerial& serial);
#endif
#if defined(ESP_PLATFORM)
    /*!
      @brief Adding unit to be managed (UART, ESP-IDF native driver)
      @param u Unit Component
      @param uart_num UART port number (the driver must be installed beforehand via
                      uart_driver_install / uart_param_config / uart_set_pin)
      @return True if successful
    */
    bool add(Component& u, const uart_port_t uart_num);
#endif
    ///@}

    ///@name Add unit(SPI)
    ///@{
#if defined(ARDUINO)
    /*!
      @brief Add unit to be managed (SPI)
      @param u Unit Component
      @param spi SPIClass to be used
      @param settings SPI settings to be applied
      @return True if successful
    */
    bool add(Component& u, SPIClass& spi, const SPISettings& settings);
#endif
    ///@}

    ///@name Add unit(M5HAL)
    ///@{
    /*!
      @brief Add unit to be managed (M5HAL bus)
      @param u Unit Component
      @param bus Bus to be used
      @return True if successful
     */
    bool add(Component& u, m5::hal::bus::Bus* bus);
    ///@}

    /*!
      @brief Begin all units under management
      @return True if all units began successfully
    */
    bool begin();
    /*!
      @brief Update all units under management
      @param force Forced communication for updates if true
    */
    void update(const bool force = false);

    /*!
      @brief Output information for debug
      @return String containing debug information
    */
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
