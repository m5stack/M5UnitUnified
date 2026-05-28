/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file m5_unit_unified_wiring.hpp
  @brief Opt-in, header-only board-aware connection helpers for M5UnitUnified examples
  @note Include this LAST. It self-includes the Arduino cores (Wire/SPI/HardwareSerial) and detects
        M5Unified/M5GFX/M5HAL via guard macros without including them.
*/
#ifndef M5_UNIT_UNIFIED_WIRING_HPP
#define M5_UNIT_UNIFIED_WIRING_HPP

#include "../M5UnitUnified.hpp"  // core (UnitUnified / Component, brings in M5HAL)
#if defined(ARDUINO)
#include <Wire.h>
#include <SPI.h>
#include <HardwareSerial.h>
#endif
// NOTE: M5Unified.h / M5GFX are NOT included; detected via __M5UNIFIED_HPP__ / __M5_I2C_CLASS_H__.

namespace m5 {
namespace unit {
namespace wiring {

///@name I2C (low-level builders)
///@{
#if defined(ARDUINO)
//! @brief Add a unit on an explicit TwoWire bus (re-inits the bus on the given pins)
inline bool i2cWire(UnitUnified& units, Component& unit, TwoWire& wire, const int sda, const int scl,
                    const uint32_t clock)
{
    M5_LIB_LOGI("wiring: I2C(Wire) sda=%d scl=%d clock=%lu", sda, scl, (unsigned long)clock);
    wire.end();
    wire.begin(sda, scl, clock);
    return units.add(unit, wire);
}
#endif

#if defined(__M5_I2C_CLASS_H__)
//! @brief Add a unit on an m5::I2C_Class bus (e.g. M5.In_I2C / M5.Ex_I2C)
inline bool i2cClass(UnitUnified& units, Component& unit, m5::I2C_Class& i2c)
{
    M5_LIB_LOGI("wiring: I2C(I2C_Class)");
    i2c.begin();
    return units.add(unit, i2c);
}
#endif

#if defined(M5_HAL_HPP)
//! @brief Add a unit on a software (bit-bang) I2C bus via M5HAL on explicit pins
inline bool i2cSoftware(UnitUnified& units, Component& unit, const int sda, const int scl)
{
    M5_LIB_LOGI("wiring: I2C(Software/M5HAL) sda=%d scl=%d", sda, scl);
    m5::hal::bus::I2CBusConfig cfg;
    cfg.pin_sda = m5::hal::gpio::getPin(sda);
    cfg.pin_scl = m5::hal::gpio::getPin(scl);
    auto bus    = m5::hal::bus::i2c::getBus(cfg);
    return units.add(unit, bus ? bus.value() : nullptr);
}
#endif
///@}

///@name I2C (high-level board-aware)
///@{
#if defined(__M5UNIFIED_HPP__)  // ⇒ ARDUINO also defined; Wire / I2C_Class / M5HAL all available
//! @brief NessoN1 connection choice: PortB = direct GROVE (SoftwareI2C), PortA = PbHub / QWIIC (Wire)
enum class NessoPort { PortB, PortA };

/*!
  @brief Add a unit on the board's default I2C, picking the right backend automatically
  @param units UnitUnified manager
  @param unit Unit Component to add
  @param clock I2C clock in Hz
  @param nesso NessoN1 only: caller picks the port. PortB = direct GROVE (SoftwareI2C, for normal
               units); PortA = QWIIC / PbHub (hardware Wire, e.g. NFC).
  @return True if successful
  @note NessoN1: PortB -> i2cSoftware (M5HAL, C6 GROVE can't use Wire), PortA -> i2cWire (Wire on
        port_a). NanoC6 -> M5.Ex_I2C (I2C_Class). Other boards -> i2cWire (Wire on port_a).
*/
inline bool addI2C(UnitUnified& units, Component& unit, const uint32_t clock = 100000,
                   const NessoPort nesso = NessoPort::PortB)
{
    const auto board = M5.getBoard();
    M5_LIB_LOGI("wiring: addI2C board=0x%02x nesso=%d", (int)board, (int)nesso);
    if (board == m5::board_t::board_ArduinoNessoN1) {
        if (nesso == NessoPort::PortB) {
            // Direct GROVE on C6: Wire can't drive it -> SoftwareI2C (M5HAL)
            return i2cSoftware(units, unit, M5.getPin(m5::pin_name_t::port_b_out),
                               M5.getPin(m5::pin_name_t::port_b_in));
        }
        // PortA (QWIIC / PbHub): hardware Wire works (matches M5Unit-NFC)
        return i2cWire(units, unit, Wire, M5.getPin(m5::pin_name_t::port_a_sda), M5.getPin(m5::pin_name_t::port_a_scl),
                       clock);
    }
    if (board == m5::board_t::board_M5NanoC6) {
        return i2cClass(units, unit, M5.Ex_I2C);
    }
    return i2cWire(units, unit, Wire, M5.getPin(m5::pin_name_t::port_a_sda), M5.getPin(m5::pin_name_t::port_a_scl),
                   clock);
}
#endif
///@}

///@name GPIO (high-level board-aware)
///@{
#if defined(__M5UNIFIED_HPP__)
/*!
  @brief Add a unit on GPIO, preferring PortB and falling back to PortA
  @param units UnitUnified manager
  @param unit Unit Component to add
  @return True if successful
  @note When PortB is unavailable, Wire.end() is called before reusing PortA's pins.
*/
inline bool addGPIO(UnitUnified& units, Component& unit)
{
    auto rx = M5.getPin(m5::pin_name_t::port_b_in);
    auto tx = M5.getPin(m5::pin_name_t::port_b_out);
    if (rx < 0 || tx < 0) {
        Wire.end();
        rx = M5.getPin(m5::pin_name_t::port_a_pin1);
        tx = M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    M5_LIB_LOGI("wiring: GPIO rx=%d tx=%d", (int)rx, (int)tx);
    return units.add(unit, static_cast<int8_t>(rx), static_cast<int8_t>(tx));
}
#endif
///@}

///@name UART
///@{
#if defined(ARDUINO)
//! @brief The board's default Serial for a Port unit, chosen by SoC UART count
inline HardwareSerial& defaultUartSerial()
{
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    return Serial1;
#elif SOC_UART_NUM > 2
    return Serial2;
#elif SOC_UART_NUM > 1
    return Serial1;
#else
#error "Not enough Serial"
#endif
}
#endif

#if defined(__M5UNIFIED_HPP__)
/*!
  @brief Add a unit on UART, preferring PortC and falling back to PortA
  @param units UnitUnified manager
  @param unit Unit Component to add
  @param serial HardwareSerial to begin and use
  @param baud Baud rate
  @param config Serial config (e.g. SERIAL_8N1)
  @return True if successful
  @note On NanoC6, M5.Ex_I2C.release() is called before using PortA: M5.begin() attaches the m5gfx
        I2C peripheral to those external pins, which would otherwise conflict with UART. No Wire.end()
        is called (unlike addI2C) -- UART uses Serial, not Wire, so the internal I2C bus is left intact.
*/
inline bool addUART(UnitUnified& units, Component& unit, HardwareSerial& serial, const uint32_t baud = 115200,
                    const uint32_t config = SERIAL_8N1)
{
    auto rx = M5.getPin(m5::pin_name_t::port_c_rxd);
    auto tx = M5.getPin(m5::pin_name_t::port_c_txd);
    if (rx < 0 || tx < 0) {
        if (M5.getBoard() == m5::board_t::board_M5NanoC6) {
            M5.Ex_I2C.release();  // free the m5gfx I2C peripheral M5.begin() attached to these pins
        }
        rx = M5.getPin(m5::pin_name_t::port_a_pin1);
        tx = M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    M5_LIB_LOGI("wiring: UART rx=%d tx=%d baud=%lu", (int)rx, (int)tx, (unsigned long)baud);
    serial.begin(baud, config, rx, tx);
    return units.add(unit, serial);
}
#endif
///@}

///@name SPI
///@{
#if defined(ARDUINO)
//! @brief Add a unit on an already-begun SPI bus (CS is taken from the unit ctor / address)
inline bool spiBus(UnitUnified& units, Component& unit, SPIClass& spi, const SPISettings& settings)
{
    M5_LIB_LOGI("wiring: SPI");
    return units.add(unit, spi, settings);
}
#endif

#if defined(__M5UNIFIED_HPP__)  // dereferences M5 -> needs M5Unified (which implies ARDUINO, so SPI exists)
/*!
  @brief Add a unit on the board's shared SD/SPI bus, beginning it on demand
  @param units UnitUnified manager
  @param unit Unit Component to add
  @param settings SPI settings (clock, bit order, data mode)
  @return True if successful
  @note Resolves the sd_spi_* pins (the M5 shared SPI bus). SPI.begin() is called only if the bus is
        not already begun. CS is taken from the unit ctor / address (this library drives it).
*/
inline bool addSPI(UnitUnified& units, Component& unit, const SPISettings& settings)
{
    const auto sclk = M5.getPin(m5::pin_name_t::sd_spi_sclk);
    const auto mosi = M5.getPin(m5::pin_name_t::sd_spi_mosi);
    const auto miso = M5.getPin(m5::pin_name_t::sd_spi_miso);
    M5_LIB_LOGI("wiring: addSPI sclk=%d miso=%d mosi=%d", (int)sclk, (int)miso, (int)mosi);
    if (!SPI.bus()) {
        SPI.begin(sclk, miso, mosi);
    }
    return spiBus(units, unit, SPI, settings);
}
#endif
///@}

///@note Hat pin tables are survey-derived; re-verify per Hat product before relying on them.
///@name Hat (2-pin header, board-aware)
///@{
#if defined(__M5UNIFIED_HPP__)
/*!
  @brief Add a unit on the board's Hat I2C header (NessoN1 uses Wire1, others Wire)
  @param units UnitUnified manager
  @param unit Unit Component to add
  @param clock I2C clock in Hz
  @return True if successful
  @note Some Hats need extra pin pre-setup (e.g. HatHEART does pinMode(scl, OUTPUT)); do that in the
        caller before this call, or build the connection with the low-level i2cWire.
*/
inline bool addHatI2C(UnitUnified& units, Component& unit, const uint32_t clock = 400000)
{
    const auto board = M5.getBoard();
    int sda = -1, scl = -1;
    switch (board) {
        case m5::board_t::board_M5StickC:
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            sda = 0;
            scl = 26;
            break;
        case m5::board_t::board_M5StickS3:
            sda = 8;
            scl = 0;
            break;
        case m5::board_t::board_M5StackCoreInk:
            sda = 25;
            scl = 26;
            break;
        case m5::board_t::board_ArduinoNessoN1:
            sda = 6;
            scl = 7;
            break;
        default:
            M5_LIB_LOGE("wiring: Hat I2C unsupported board=0x%02x", (int)board);
            return false;
    }
    const bool useWire1 = (board == m5::board_t::board_ArduinoNessoN1);
    TwoWire& wire       = useWire1 ? Wire1 : Wire;
    M5_LIB_LOGI("wiring: addHatI2C board=0x%02x sda=%d scl=%d clock=%lu wire=%s", (int)board, sda, scl,
                (unsigned long)clock, useWire1 ? "Wire1" : "Wire");
    return i2cWire(units, unit, wire, sda, scl, clock);
}

/*!
  @brief Add a unit on the board's Hat GPIO header
  @param units UnitUnified manager
  @param unit Unit Component to add
  @return True if successful
*/
inline bool addHatGPIO(UnitUnified& units, Component& unit)
{
    const auto board = M5.getBoard();
    int rx = -1, tx = -1;
    switch (board) {
        case m5::board_t::board_M5StickC:
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            rx = 26;
            tx = 0;
            break;
        case m5::board_t::board_M5StickS3:
            rx = 0;
            tx = 8;
            break;
        case m5::board_t::board_M5StackCoreInk:
            rx = 26;
            tx = 25;
            break;
        case m5::board_t::board_ArduinoNessoN1:
            rx = 5;
            tx = 4;
            break;
        default:
            M5_LIB_LOGE("wiring: Hat GPIO unsupported board=0x%02x", (int)board);
            return false;
    }
    M5_LIB_LOGI("wiring: addHatGPIO board=0x%02x rx=%d tx=%d", (int)board, rx, tx);
    return units.add(unit, static_cast<int8_t>(rx), static_cast<int8_t>(tx));
}

/*!
  @brief Add a unit on the board's Hat UART header
  @param units UnitUnified manager
  @param unit Unit Component to add
  @param serial HardwareSerial to begin and use
  @param baud Baud rate
  @param config Serial config (e.g. SERIAL_8N1)
  @return True if successful
*/
inline bool addHatUART(UnitUnified& units, Component& unit, HardwareSerial& serial, const uint32_t baud = 115200,
                       const uint32_t config = SERIAL_8N1)
{
    const auto board = M5.getBoard();
    int rx = -1, tx = -1;
    switch (board) {
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            rx = 26;
            tx = 0;
            break;
        case m5::board_t::board_M5StickS3:
            rx = 0;
            tx = 8;
            break;
        case m5::board_t::board_M5StackCoreInk:
            rx = 26;
            tx = 25;
            break;
        case m5::board_t::board_ArduinoNessoN1:
            rx = 7;
            tx = 6;
            break;
        default:
            M5_LIB_LOGE("wiring: Hat UART unsupported board=0x%02x", (int)board);
            return false;
    }
    M5_LIB_LOGI("wiring: addHatUART board=0x%02x rx=%d tx=%d baud=%lu", (int)board, rx, tx, (unsigned long)baud);
    serial.begin(baud, config, rx, tx);
    return units.add(unit, serial);
}
#endif
///@}

///@name Error handling
///@{
#if defined(__M5UNIFIED_HPP__)
//! @brief Fill the display red and halt (for begin() failure in examples)
[[noreturn]] inline void failStop()
{
    M5.Display.fillScreen(0xF800 /* TFT_RED */);
    while (true) {
        m5::utility::delay(10000);
    }
}
#endif
///@}

}  // namespace wiring
}  // namespace unit
}  // namespace m5
#endif  // M5_UNIT_UNIFIED_WIRING_HPP
