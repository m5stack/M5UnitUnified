/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file m5_unit_unified_wiring.hpp
  @brief Opt-in, header-only board-aware connection helpers for M5UnitUnified examples

  @details
  This header exists solely to keep the example code in M5UnitUnified and M5Unit-* repositories
  short and readable. Each helper resolves board-specific pins / backends through M5Unified,
  opens the bus, and calls UnitUnified::add().

  Users SHOULD treat these helpers as reference implementations, not as the only supported way
  to wire a unit. In real applications, prefer to call UnitUnified::add() directly with a bus
  or handle that fits the application's lifecycle (shared bus management, custom pins,
  alternative backends, etc.).

  @note REQUIRES <M5Unified.h> (or <M5Unified.hpp>) to be included BEFORE this header.
        Pin-acquisition helpers (i2cPins / gpioPins / uartPins / spiPins / hatI2CPins / hatGPIOPins /
        hatUARTPins) work on both Arduino and ESP-IDF (any platform where M5Unified is available).
        High-level addX helpers (addI2C / addGPIO / addUART / addSPI / addHatI2C / addHatGPIO /
        addHatUART) provide the same signatures on Arduino and ESP-IDF native, so example code can
        compile on both frameworks unchanged.

  @note On ESP-IDF native, the following public lifecycle hooks are also exposed for unit-specific
        wiring developers (M5Unit-* libraries that need to share a bus handle with the generic
        helpers above): `i2cBusHandle`, `uartPortHandle`, `spiDeviceHandle`. Examples should NOT
        use these directly; use the generic addX helpers instead.
*/
#ifndef M5_UNIT_UNIFIED_WIRING_HPP
#define M5_UNIT_UNIFIED_WIRING_HPP

#include "../M5UnitUnified.hpp"  // core (UnitUnified / Component, brings in M5HAL)
#if defined(ARDUINO)
#include <Wire.h>
#include <SPI.h>
#include <HardwareSerial.h>
#else  // ESP-IDF native
// driver/i2c_master.h (new I2C master driver) exists only on IDF >= 5.2. On IDF 5.0/5.1 fall back to
// the legacy driver/i2c.h so the I2C wiring helpers keep working across the whole IDF >= 5.0 range.
#if __has_include(<driver/i2c_master.h>)
#include <driver/i2c_master.h>
#else
#include <driver/i2c.h>
#endif
#include <driver/uart.h>
#include <driver/spi_master.h>
#include <hal/gpio_types.h>
#include <soc/soc_caps.h>
#endif
// NOTE: M5Unified.h / M5GFX are NOT included; caller must include them BEFORE this header.

namespace m5 {
namespace unit {
namespace wiring {

///@name Pin acquisition options (enums, no platform dependency)
///@{
//! @brief NessoN1 connection choice: PortB = direct GROVE (SoftwareI2C), PortA = QWIIC / PbHub (Wire)
enum class NessoPort { PortB, PortA };

//! @brief GPIO usage role: how many pins of the port the unit actually uses.
enum class GpioRole {
    Both,    //!< Both input and output pins (e.g., IR Tx+Rx)
    InOnly,  //!< Input only — rx = port_*_in (pin1), tx = -1 (e.g., RF433 Rx, IR Rx alone)
    OutOnly  //!< Output only — rx = -1, tx = port_*_out (pin2) (e.g., Flashlight, RF433 Tx, IR Tx alone)
};

//! @brief UART config (word length / parity / stop bits). Common across Arduino and ESP-IDF native.
enum class UartConfig : uint32_t {
    Default = 0,  //!< = _8N1
    _8N1    = 1,
    _8N2    = 2,
    _8E1    = 3,
    _8O1    = 4,
    _7N1    = 5,
    _7E1    = 6,
    _7O1    = 7,
};
///@}

///@name Pin acquisition result structs (pure data, no platform dependency)
///@{
//! @brief I2C backend selection + pin numbers (pure getter result, no side effects)
struct I2CPins {
    int8_t sda;
    int8_t scl;
    enum class Backend : uint8_t {
        Wire,         //!< Arduino TwoWire on (sda, scl)
        SoftwareI2C,  //!< M5HAL SoftwareI2C on (sda, scl) — NessoN1 PortB
        ExI2C         //!< M5.Ex_I2C (pin numbers are for reference; M5Unified manages them)
    } backend;
};

//! @brief GPIO rx/tx pin pair; fallback_a indicates port_a fallback was used
struct GpioPinPair {
    int8_t rx;
    int8_t tx;
    bool fallback_a;  //!< True when PortB unavailable and PortA was chosen
};

//! @brief UART rx/tx pin pair; fallback_a indicates port_a fallback was used
struct UartPinPair {
    int8_t rx;
    int8_t tx;
    bool fallback_a;  //!< True when PortC unavailable and PortA was chosen
};

//! @brief SPI pin set (sclk / miso / mosi) on the board's shared SD/SPI bus
struct SpiPins {
    int8_t sclk;
    int8_t miso;
    int8_t mosi;
};

//! @brief Hat I2C pin pair + bus selection; sda=scl=-1 if board has no Hat header
struct HatI2CPins {
    int8_t sda;
    int8_t scl;
    bool useWire1;  //!< True if the Hat uses Wire1 (NessoN1); else use Wire
};

//! @brief Hat rx/tx pin pair (shared by Hat GPIO and Hat UART); rx=tx=-1 if board has no Hat header
struct HatPinPair {
    int8_t rx;
    int8_t tx;
};
///@}

///@name Pin acquisition helpers (pure getters — usable on Arduino AND ESP-IDF when M5Unified included)
///@{
#if defined(__M5UNIFIED_HPP__)  // M5Unified.h must be included before this header
/*!
  @brief Choose I2C backend + pins for the board's default I2C port.
  @param nesso NessoN1 only: PortB = direct GROVE (SoftwareI2C), PortA = QWIIC (Wire)
  @return I2CPins { sda, scl, backend }
  @note NessoN1 PortB -> SoftwareI2C on port_b. NessoN1 PortA -> Wire on port_a (= QWIIC).
        NanoC6/NanoH2 -> ExI2C (sda/scl reflect port_a pins for diagnostic). Others -> Wire on port_a.
*/
inline I2CPins i2cPins(const NessoPort nesso = NessoPort::PortB)
{
    const auto board = M5.getBoard();
    if (board == m5::board_t::board_ArduinoNessoN1) {
        if (nesso == NessoPort::PortB) {
            return {static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_b_out)),
                    static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_b_in)), I2CPins::Backend::SoftwareI2C};
        }
        return {static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_a_sda)),
                static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_a_scl)), I2CPins::Backend::Wire};
    }
    if (board == m5::board_t::board_M5NanoC6 || board == m5::board_t::board_M5NanoH2) {
        return {static_cast<int8_t>(M5.getPin(m5::pin_name_t::ex_i2c_sda)),
                static_cast<int8_t>(M5.getPin(m5::pin_name_t::ex_i2c_scl)), I2CPins::Backend::ExI2C};
    }
    return {static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_a_sda)),
            static_cast<int8_t>(M5.getPin(m5::pin_name_t::port_a_scl)), I2CPins::Backend::Wire};
}

/*!
  @brief Choose GPIO pins (rx, tx) for the role; PortB preferred, PortA fallback.
  @param role Which pin(s) of the port the unit uses (default: Both)
  @return GpioPinPair { rx, tx, fallback_a }
  @note Pure: does NOT call Wire.end(). The caller (addGPIO) is responsible for the side effect
        when fallback_a is true (port_a pins typically shared with Wire).
*/
inline GpioPinPair gpioPins(const GpioRole role = GpioRole::Both)
{
    int rx                = (role == GpioRole::OutOnly) ? -1 : M5.getPin(m5::pin_name_t::port_b_in);
    int tx                = (role == GpioRole::InOnly) ? -1 : M5.getPin(m5::pin_name_t::port_b_out);
    const bool need_rx_fb = (role != GpioRole::OutOnly && rx < 0);
    const bool need_tx_fb = (role != GpioRole::InOnly && tx < 0);
    bool fb               = false;
    if (need_rx_fb || need_tx_fb) {
        fb = true;
        rx = (role == GpioRole::OutOnly) ? -1 : M5.getPin(m5::pin_name_t::port_a_pin1);
        tx = (role == GpioRole::InOnly) ? -1 : M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    return {static_cast<int8_t>(rx), static_cast<int8_t>(tx), fb};
}

/*!
  @brief Choose UART pins (rx, tx) for the board; PortC preferred, PortA fallback.
  @return UartPinPair { rx, tx, fallback_a }
  @note Pure: does NOT release Ex_I2C on NanoC6/NanoH2. The caller (addUART) handles that side
        effect when fallback_a is true on those boards.
*/
inline UartPinPair uartPins()
{
    int rx  = M5.getPin(m5::pin_name_t::port_c_rxd);
    int tx  = M5.getPin(m5::pin_name_t::port_c_txd);
    bool fb = false;
    if (rx < 0 || tx < 0) {
        fb = true;
        rx = M5.getPin(m5::pin_name_t::port_a_pin1);
        tx = M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    return {static_cast<int8_t>(rx), static_cast<int8_t>(tx), fb};
}

/*!
  @brief SPI pins for the board's shared SD/SPI bus.
  @return SpiPins { sclk, miso, mosi }
*/
inline SpiPins spiPins()
{
    return {static_cast<int8_t>(M5.getPin(m5::pin_name_t::sd_spi_sclk)),
            static_cast<int8_t>(M5.getPin(m5::pin_name_t::sd_spi_miso)),
            static_cast<int8_t>(M5.getPin(m5::pin_name_t::sd_spi_mosi))};
}

/*!
  @brief Hat I2C pins + bus selection. Returns sda=scl=-1 on boards without a Hat header.
  @note Hat pin tables are survey-derived; re-verify per Hat product before relying on them.
*/
inline HatI2CPins hatI2CPins()
{
    const auto board = M5.getBoard();
    switch (board) {
        case m5::board_t::board_M5StickC:
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            return {0, 26, false};
        case m5::board_t::board_M5StickS3:
            return {8, 0, false};
        case m5::board_t::board_M5StackCoreInk:
            return {25, 26, false};
        case m5::board_t::board_ArduinoNessoN1:
            return {6, 7, true};  // useWire1
        default:
            return {-1, -1, false};
    }
}

/*!
  @brief Hat GPIO pins (rx, tx). Returns rx=tx=-1 on boards without a Hat header.
  @note NessoN1 Hat header GPIO 6/7 shared with Wire1 (I2C) / UART (role-swapped per usage).
*/
inline HatPinPair hatGPIOPins()
{
    const auto board = M5.getBoard();
    switch (board) {
        case m5::board_t::board_M5StickC:
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            return {26, 0};
        case m5::board_t::board_M5StickS3:
            return {0, 8};
        case m5::board_t::board_M5StackCoreInk:
            return {26, 25};
        case m5::board_t::board_ArduinoNessoN1:
            return {7, 6};
        default:
            return {-1, -1};
    }
}

/*!
  @brief Hat UART pins (rx, tx). Returns rx=tx=-1 on boards without a Hat UART header.
*/
inline HatPinPair hatUARTPins()
{
    const auto board = M5.getBoard();
    switch (board) {
        case m5::board_t::board_M5StickCPlus:
        case m5::board_t::board_M5StickCPlus2:
            return {26, 0};
        case m5::board_t::board_M5StickS3:
            return {0, 8};
        case m5::board_t::board_M5StackCoreInk:
            return {26, 25};
        case m5::board_t::board_ArduinoNessoN1:
            return {7, 6};
        default:
            return {-1, -1};
    }
}
#endif  // __M5UNIFIED_HPP__
///@}

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

///@name High-level addX (Arduino only — drives Wire / SPI / HardwareSerial)
///@{
#if defined(ARDUINO)
/*!
  @brief Add a unit on the board's default I2C, picking the right backend automatically
  @param nesso NessoN1 only: PortB (default) -> SoftwareI2C, PortA -> Wire (QWIIC)
  @note Dispatches via i2cPins() backend: Wire / SoftwareI2C / ExI2C.
*/
inline bool addI2C(UnitUnified& units, Component& unit, const uint32_t clock = 100000,
                   const NessoPort nesso = NessoPort::PortB)
{
    const auto p = i2cPins(nesso);
    M5_LIB_LOGI("wiring: addI2C board=0x%02x nesso=%d backend=%d sda=%d scl=%d", (int)M5.getBoard(), (int)nesso,
                (int)p.backend, (int)p.sda, (int)p.scl);
    switch (p.backend) {
        case I2CPins::Backend::SoftwareI2C:
            return i2cSoftware(units, unit, p.sda, p.scl);
        case I2CPins::Backend::Wire:
            return i2cWire(units, unit, Wire, p.sda, p.scl, clock);
        case I2CPins::Backend::ExI2C:
            return i2cClass(units, unit, M5.Ex_I2C);
    }
    return false;
}

/*!
  @brief Add a unit on GPIO, preferring PortB and falling back to PortA
  @note When fallback to PortA, Wire.end() is called to release the pins (port_a typically shared
        with Wire). Pins not needed by the role are passed as -1 to keep the unit's adapter from
        claiming them.
*/
inline bool addGPIO(UnitUnified& units, Component& unit, const GpioRole role = GpioRole::Both)
{
    const auto p = gpioPins(role);
    if (p.fallback_a) {
        Wire.end();
    }
    M5_LIB_LOGI("wiring: GPIO rx=%d tx=%d role=%d fallback_a=%d", (int)p.rx, (int)p.tx, (int)role, (int)p.fallback_a);
    return units.add(unit, p.rx, p.tx);
}

//! @brief Map UartConfig to Arduino's SERIAL_* uint32_t value
inline uint32_t toArduinoSerialConfig(const UartConfig c)
{
    switch (c) {
        case UartConfig::_8N2:
            return SERIAL_8N2;
        case UartConfig::_8E1:
            return SERIAL_8E1;
        case UartConfig::_8O1:
            return SERIAL_8O1;
        case UartConfig::_7N1:
            return SERIAL_7N1;
        case UartConfig::_7E1:
            return SERIAL_7E1;
        case UartConfig::_7O1:
            return SERIAL_7O1;
        case UartConfig::Default:
        case UartConfig::_8N1:
        default:
            return SERIAL_8N1;
    }
}

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

/*!
  @brief Add a unit on the board's default UART, preferring PortC and falling back to PortA
  @param baud Baud rate (default 115200)
  @param config UartConfig enum (default `UartConfig::Default` = 8N1)
  @note Selects HardwareSerial via defaultUartSerial() based on board.
        On NanoC6 / NanoH2 fallback, M5.Ex_I2C.begin() + release() is called to flip the m5gfx
        I2C `initialized` flag so release() actually detaches the bus + restores pins.
*/
inline bool addUART(UnitUnified& units, Component& unit, const uint32_t baud = 115200,
                    const UartConfig config = UartConfig::Default)
{
    HardwareSerial& serial = defaultUartSerial();
    const uint32_t cfg     = toArduinoSerialConfig(config);
    const auto p           = uartPins();
    if (p.fallback_a) {
        const auto b = M5.getBoard();
        if (b == m5::board_t::board_M5NanoC6 || b == m5::board_t::board_M5NanoH2) {
            // M5.begin() calls Ex_I2C.setPort() but NOT begin(). m5gfx::i2c::release() is gated
            // by the internal `initialized` flag, so release() alone is a no-op. Workaround:
            // begin() flips initialized=true, then release() actually detaches + restores pins.
            M5.Ex_I2C.begin();
            M5.Ex_I2C.release();
        }
    }
    M5_LIB_LOGI("wiring: UART rx=%d tx=%d baud=%lu config=0x%lx fallback_a=%d", (int)p.rx, (int)p.tx,
                (unsigned long)baud, (unsigned long)cfg, (int)p.fallback_a);
    serial.end();
    serial.begin(baud, cfg, p.rx, p.tx);
    return units.add(unit, serial);
}

//! @brief Add a unit on an already-begun SPI bus (CS is taken from the unit ctor / address)
inline bool spiBus(UnitUnified& units, Component& unit, SPIClass& spi, const SPISettings& settings)
{
    M5_LIB_LOGI("wiring: SPI");
    return units.add(unit, spi, settings);
}

/*!
  @brief Add a unit on the board's shared SD/SPI bus, beginning it on demand
  @param clock_hz SPI clock in Hz
  @param mode SPI mode 0/1/2/3 (default 0)
  @param bit_order Bit order: 0 = MSBFIRST (default), 1 = LSBFIRST
  @note Resolves the sd_spi_* pins. SPI.begin() is called only if the bus is not already begun.
*/
inline bool addSPI(UnitUnified& units, Component& unit, const uint32_t clock_hz, const uint8_t mode = 0,
                   const uint8_t bit_order = 0)
{
    SPISettings settings{clock_hz, static_cast<uint8_t>((bit_order == 0) ? MSBFIRST : LSBFIRST), mode};
    const auto p = spiPins();
    M5_LIB_LOGI("wiring: addSPI sclk=%d miso=%d mosi=%d clock=%lu mode=%u bit_order=%u", (int)p.sclk, (int)p.miso,
                (int)p.mosi, (unsigned long)clock_hz, (unsigned)mode, (unsigned)bit_order);
    if (!SPI.bus()) {
        SPI.begin(p.sclk, p.miso, p.mosi);
    }
    return spiBus(units, unit, SPI, settings);
}

///@note Hat pin tables are survey-derived; re-verify per Hat product before relying on them.

/*!
  @brief Add a unit on the board's Hat I2C header (NessoN1 uses Wire1, others Wire)
  @note Some Hats need extra pin pre-setup (e.g. HatHEART does pinMode(scl, OUTPUT)); do that in the
        caller before this call, or build the connection with the low-level i2cWire.
*/
inline bool addHatI2C(UnitUnified& units, Component& unit, const uint32_t clock = 400000)
{
    const auto p = hatI2CPins();
    if (p.sda < 0 || p.scl < 0) {
        M5_LIB_LOGE("wiring: Hat I2C unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
#if SOC_I2C_NUM > 1
    TwoWire& wire = p.useWire1 ? Wire1 : Wire;
#else
    // SOC_I2C_NUM == 1 (ESP32-C3/C6/H2): Arduino-ESP32 declares Wire only; Wire1 is absent.
    if (p.useWire1) {
        M5_LIB_LOGE("wiring: addHatI2C NessoN1 Hat needs Wire1, but SOC_I2C_NUM==1");
        return false;
    }
    TwoWire& wire = Wire;
#endif
    M5_LIB_LOGI("wiring: addHatI2C board=0x%02x sda=%d scl=%d clock=%lu wire=%s", (int)M5.getBoard(), (int)p.sda,
                (int)p.scl, (unsigned long)clock, p.useWire1 ? "Wire1" : "Wire");
    return i2cWire(units, unit, wire, p.sda, p.scl, clock);
}

/*!
  @brief Add a unit on the board's Hat GPIO header
  @note Pins not needed by the role are passed as -1 to keep the unit's adapter from claiming them.
*/
inline bool addHatGPIO(UnitUnified& units, Component& unit, const GpioRole role = GpioRole::Both)
{
    auto p = hatGPIOPins();
    if (p.rx < 0 || p.tx < 0) {
        M5_LIB_LOGE("wiring: Hat GPIO unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
    if (role == GpioRole::OutOnly) p.rx = -1;
    if (role == GpioRole::InOnly) p.tx = -1;
    M5_LIB_LOGI("wiring: addHatGPIO board=0x%02x rx=%d tx=%d role=%d", (int)M5.getBoard(), (int)p.rx, (int)p.tx,
                (int)role);
    return units.add(unit, p.rx, p.tx);
}

/*!
  @brief Add a unit on the board's Hat UART header
  @param baud Baud rate (default 115200)
  @param config UartConfig enum (default `UartConfig::Default` = 8N1)
  @note Selects HardwareSerial via defaultUartSerial() based on board.
*/
inline bool addHatUART(UnitUnified& units, Component& unit, const uint32_t baud = 115200,
                       const UartConfig config = UartConfig::Default)
{
    HardwareSerial& serial = defaultUartSerial();
    const uint32_t cfg     = toArduinoSerialConfig(config);
    const auto p           = hatUARTPins();
    if (p.rx < 0 || p.tx < 0) {
        M5_LIB_LOGE("wiring: Hat UART unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
    M5_LIB_LOGI("wiring: addHatUART board=0x%02x rx=%d tx=%d baud=%lu config=0x%lx", (int)M5.getBoard(), (int)p.rx,
                (int)p.tx, (unsigned long)baud, (unsigned long)cfg);
    serial.end();
    serial.begin(baud, cfg, p.rx, p.tx);
    return units.add(unit, serial);
}
#endif  // ARDUINO

///@name ESP-IDF native infrastructure (cache, hooks, defaults)
///@{
#if !defined(ARDUINO)

//! @brief The board's default UART port for a Port unit, chosen by SoC UART count (ESP-IDF native)
inline uart_port_t defaultUartPort()
{
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    return UART_NUM_1;
#elif SOC_UART_NUM > 2
    return UART_NUM_2;
#elif SOC_UART_NUM > 1
    return UART_NUM_1;
#else
#error "Not enough UART"
#endif
}

//! @brief Map UartConfig to ESP-IDF native uart_config_t fields
struct IdfUartParams {
    uart_word_length_t word_length;
    uart_parity_t parity;
    uart_stop_bits_t stop_bits;
};
inline IdfUartParams toIdfUartParams(const UartConfig c)
{
    switch (c) {
        case UartConfig::_8N2:
            return {UART_DATA_8_BITS, UART_PARITY_DISABLE, UART_STOP_BITS_2};
        case UartConfig::_8E1:
            return {UART_DATA_8_BITS, UART_PARITY_EVEN, UART_STOP_BITS_1};
        case UartConfig::_8O1:
            return {UART_DATA_8_BITS, UART_PARITY_ODD, UART_STOP_BITS_1};
        case UartConfig::_7N1:
            return {UART_DATA_7_BITS, UART_PARITY_DISABLE, UART_STOP_BITS_1};
        case UartConfig::_7E1:
            return {UART_DATA_7_BITS, UART_PARITY_EVEN, UART_STOP_BITS_1};
        case UartConfig::_7O1:
            return {UART_DATA_7_BITS, UART_PARITY_ODD, UART_STOP_BITS_1};
        case UartConfig::Default:
        case UartConfig::_8N1:
        default:
            return {UART_DATA_8_BITS, UART_PARITY_DISABLE, UART_STOP_BITS_1};
    }
}

namespace detail {

constexpr size_t kI2CBusCacheSize   = 4;
constexpr size_t kUARTPortCacheSize = 4;
constexpr size_t kSPIHostCacheSize  = 2;  // SPI2_HOST / SPI3_HOST
constexpr size_t kSPIDevCacheSize   = 4;

#if __has_include(<driver/i2c_master.h>)
struct I2CCacheEntry {
    uint32_t key;
    i2c_master_bus_handle_t handle;
};
#endif

struct UARTCacheEntry {
    uint32_t key;
    uart_port_t port;
};

struct SPIHostEntry {
    spi_host_device_t host;
    bool initialized;
};

struct SPIDevEntry {
    uint32_t key;
    spi_device_handle_t handle;
};

#if __has_include(<driver/i2c_master.h>)
//! @brief Get or create an I2C master bus, cached by {port, sda, scl}.
inline i2c_master_bus_handle_t ensureI2CBus(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl,
                                            const uint32_t /*clock - not used to key, single bus per pins*/)
{
    const uint32_t key = (static_cast<uint32_t>(static_cast<uint8_t>(port)) << 24) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(sda)) << 16) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(scl)) << 8);
    static I2CCacheEntry cache[kI2CBusCacheSize]{};
    static size_t count{};
    for (size_t i = 0; i < count; ++i) {
        if (cache[i].key == key) return cache[i].handle;
    }
    if (count >= kI2CBusCacheSize) {
        M5_LIB_LOGE("wiring: I2C bus cache full (max %zu)", kI2CBusCacheSize);
        return nullptr;
    }
    i2c_master_bus_config_t cfg{};
    cfg.i2c_port   = port;
    cfg.sda_io_num = sda;
    cfg.scl_io_num = scl;
#if SOC_LP_I2C_SUPPORTED
    if (port == LP_I2C_NUM_0) {
        // LP I2C uses RTC_FAST clock (union with clk_source, mutually exclusive).
        cfg.lp_source_clk = LP_I2C_SCLK_DEFAULT;
    } else
#endif
    {
        cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    }
    cfg.glitch_ignore_cnt            = 7;
    cfg.flags.enable_internal_pullup = true;
    if (i2c_new_master_bus(&cfg, &cache[count].handle) != ESP_OK) {
        M5_LIB_LOGE("wiring: i2c_new_master_bus failed port=%d sda=%d scl=%d", (int)port, (int)sda, (int)scl);
        return nullptr;
    }
    cache[count].key = key;
    return cache[count++].handle;
}
#else  // legacy driver/i2c.h (IDF 5.0 / 5.1)
//! @brief Install the legacy I2C master driver once per {port, sda, scl}; cached so re-adds are no-ops.
inline bool ensureI2CLegacyDriver(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl,
                                  const uint32_t clock)
{
    const uint32_t key = (static_cast<uint32_t>(static_cast<uint8_t>(port)) << 24) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(sda)) << 16) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(scl)) << 8);
    static uint32_t cache[kI2CBusCacheSize]{};
    static size_t count{};
    for (size_t i = 0; i < count; ++i) {
        if (cache[i] == key) return true;
    }
    if (count >= kI2CBusCacheSize) {
        M5_LIB_LOGE("wiring: I2C bus cache full (max %zu)", kI2CBusCacheSize);
        return false;
    }
    i2c_config_t conf{};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = true;
    conf.scl_pullup_en = true;
    conf.master.clk_speed = clock;
    if (i2c_param_config(port, &conf) != ESP_OK || i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK) {
        M5_LIB_LOGE("wiring: legacy i2c driver install failed port=%d sda=%d scl=%d", (int)port, (int)sda, (int)scl);
        return false;
    }
    cache[count++] = key;
    return true;
}
#endif

//! @brief Get or install a UART port, cached by {port, rx, tx}.
inline uart_port_t ensureUARTPort(const uart_port_t port, const gpio_num_t rx, const gpio_num_t tx, const uint32_t baud,
                                  const UartConfig config)
{
    const uint32_t key = (static_cast<uint32_t>(static_cast<uint8_t>(port)) << 24) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(rx)) << 16) |
                         (static_cast<uint32_t>(static_cast<uint8_t>(tx)) << 8);
    static UARTCacheEntry cache[kUARTPortCacheSize]{};
    static size_t count{};
    for (size_t i = 0; i < count; ++i) {
        if (cache[i].key == key) return cache[i].port;
    }
    if (count >= kUARTPortCacheSize) {
        M5_LIB_LOGE("wiring: UART port cache full (max %zu)", kUARTPortCacheSize);
        return UART_NUM_MAX;
    }
    if (uart_is_driver_installed(port)) {
        cache[count] = {key, port};
        return cache[count++].port;
    }
    const auto p = toIdfUartParams(config);
    uart_config_t uc{};
    uc.baud_rate  = static_cast<int>(baud);
    uc.data_bits  = p.word_length;
    uc.parity     = p.parity;
    uc.stop_bits  = p.stop_bits;
    uc.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uc.source_clk = UART_SCLK_DEFAULT;
    if (uart_param_config(port, &uc) != ESP_OK ||
        uart_set_pin(port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK ||
        uart_driver_install(port, 256, 0, 0, nullptr, 0) != ESP_OK) {
        M5_LIB_LOGE("wiring: uart setup failed port=%d", (int)port);
        return UART_NUM_MAX;
    }
    cache[count] = {key, port};
    return cache[count++].port;
}

//! @brief Get or initialize a SPI device, cached by {host, clock_hz, mode, bit_order}.
//! @note  spics_io_num is fixed to GPIO_NUM_NC; cs is controlled manually by the unit's adapter.
inline spi_device_handle_t ensureSPIDevice(const spi_host_device_t host, const gpio_num_t mosi, const gpio_num_t miso,
                                           const gpio_num_t sck, const uint32_t clock_hz, const uint8_t mode,
                                           const uint8_t bit_order)
{
    static SPIHostEntry host_cache[kSPIHostCacheSize]{};
    static SPIDevEntry dev_cache[kSPIDevCacheSize]{};
    static size_t host_count{};
    static size_t dev_count{};

    bool host_init = false;
    for (size_t i = 0; i < host_count; ++i) {
        if (host_cache[i].host == host) {
            host_init = host_cache[i].initialized;
            break;
        }
    }
    if (!host_init) {
        spi_bus_config_t bc{};
        bc.mosi_io_num     = mosi;
        bc.miso_io_num     = miso;
        bc.sclk_io_num     = sck;
        bc.quadwp_io_num   = -1;
        bc.quadhd_io_num   = -1;
        bc.max_transfer_sz = 4096;
        if (spi_bus_initialize(host, &bc, SPI_DMA_CH_AUTO) != ESP_OK) {
            M5_LIB_LOGE("wiring: spi_bus_initialize failed host=%d", (int)host);
            return nullptr;
        }
        if (host_count < kSPIHostCacheSize) host_cache[host_count++] = {host, true};
    }

    const uint32_t key = (static_cast<uint32_t>(static_cast<uint8_t>(host)) << 24) |
                         (static_cast<uint32_t>(clock_hz / 1000U) << 8) | (static_cast<uint32_t>(mode) << 4) |
                         static_cast<uint32_t>(bit_order);
    for (size_t i = 0; i < dev_count; ++i) {
        if (dev_cache[i].key == key) return dev_cache[i].handle;
    }
    if (dev_count >= kSPIDevCacheSize) {
        M5_LIB_LOGE("wiring: SPI device cache full (max %zu)", kSPIDevCacheSize);
        return nullptr;
    }
    spi_device_interface_config_t dc{};
    dc.clock_speed_hz = static_cast<int>(clock_hz);
    dc.mode           = mode;
    dc.spics_io_num   = GPIO_NUM_NC;  // manual CS control by adapter
    dc.queue_size     = 1;
    if (bit_order == 1) dc.flags |= SPI_DEVICE_BIT_LSBFIRST;
    if (spi_bus_add_device(host, &dc, &dev_cache[dev_count].handle) != ESP_OK) {
        M5_LIB_LOGE("wiring: spi_bus_add_device failed host=%d", (int)host);
        return nullptr;
    }
    dev_cache[dev_count].key = key;
    return dev_cache[dev_count++].handle;
}

}  // namespace detail

#if __has_include(<driver/i2c_master.h>)
/*!
  @brief Get or create an I2C master bus handle (cached by {port, sda, scl})
  @note  Intended for unit-specific wiring in M5Unit-* libraries; examples should use addI2C() instead.
  @note  Available only on IDF >= 5.2 (new I2C master driver). On IDF 5.0/5.1 use addI2C() instead.
*/
inline i2c_master_bus_handle_t i2cBusHandle(const i2c_port_t port, const gpio_num_t sda, const gpio_num_t scl,
                                            const uint32_t clock = 100000)
{
    return detail::ensureI2CBus(port, sda, scl, clock);
}
#endif

/*!
  @brief Get or install a UART port (cached by {port, rx, tx})
  @note  Intended for unit-specific wiring; examples should use addUART() instead.
*/
inline uart_port_t uartPortHandle(const uart_port_t port, const gpio_num_t rx, const gpio_num_t tx, const uint32_t baud,
                                  const UartConfig config = UartConfig::Default)
{
    return detail::ensureUARTPort(port, rx, tx, baud, config);
}

/*!
  @brief Get or initialize a SPI device on the given host (cached by {host, clock_hz, mode, bit_order})
  @note  Intended for unit-specific wiring; examples should use addSPI() instead.
         CS is controlled manually by the unit's adapter (spics_io_num = GPIO_NUM_NC).
*/
inline spi_device_handle_t spiDeviceHandle(const spi_host_device_t host, const gpio_num_t mosi, const gpio_num_t miso,
                                           const gpio_num_t sck, const uint32_t clock_hz, const uint8_t mode = 0,
                                           const uint8_t bit_order = 0)
{
    return detail::ensureSPIDevice(host, mosi, miso, sck, clock_hz, mode, bit_order);
}

//
// === ESP-IDF native generic helpers (Phase C) — same signatures as Arduino ===
//

#if defined(__M5UNIFIED_HPP__)
//! @brief Add a unit on the board's default I2C, picking the right backend automatically (ESP-IDF native)
//! @note  Dispatch matrix (mirrors Arduino addI2C's intent on each backend / board):
//!          - SoftwareI2C (NessoN1 PortB)   -> i2cSoftware() via M5HAL bit-bang
//!          - Wire on NessoN1 (PortA QWIIC) -> borrow M5.Ex_I2C (M5Unified holds I2C_NUM_0)
//!          - Wire on other boards (Core/Stick/S3 etc) -> install a new bus via i2c_new_master_bus
//!            (M5Unified does NOT call Wire.begin() on these boards, so I2C_NUM_0 is free)
//!          - ExI2C (NanoC6/NanoH2)         -> borrow M5.Ex_I2C
inline bool addI2C(UnitUnified& units, Component& unit, const uint32_t clock = 100000,
                   const NessoPort nesso = NessoPort::PortB)
{
    const auto pins  = i2cPins(nesso);
    const auto board = M5.getBoard();
    M5_LIB_LOGI("wiring(ESP-IDF): addI2C board=0x%02x nesso=%d sda=%d scl=%d clock=%u backend=%d", (int)board,
                (int)nesso, (int)pins.sda, (int)pins.scl, (unsigned)clock, (int)pins.backend);
    switch (pins.backend) {
        case I2CPins::Backend::SoftwareI2C:
#if defined(M5_HAL_HPP)
            return i2cSoftware(units, unit, pins.sda, pins.scl);
#else
            M5_LIB_LOGE("wiring: SoftwareI2C backend requires M5HAL");
            return false;
#endif
        case I2CPins::Backend::Wire:
            if (board == m5::board_t::board_ArduinoNessoN1) {
                // NessoN1 PortA (QWIIC) -> Wire == M5.In_I2C (NessoN1 maps Wire to In_I2C).
                // M5Unified already holds I2C_NUM_0 here, so borrow instead of installing.
                return i2cClass(units, unit, M5.In_I2C);
            } else {
                // Core / Stick / S3 etc: M5Unified does not call Wire.begin(), so install ourselves
#if __has_include(<driver/i2c_master.h>)
                auto bus = i2cBusHandle(I2C_NUM_0, (gpio_num_t)pins.sda, (gpio_num_t)pins.scl, clock);
                if (!bus) return false;
                return units.add(unit, bus);
#else  // legacy driver (IDF 5.0 / 5.1): install legacy driver, then add by port
                if (!detail::ensureI2CLegacyDriver(I2C_NUM_0, (gpio_num_t)pins.sda, (gpio_num_t)pins.scl, clock)) {
                    return false;
                }
                return units.add(unit, I2C_NUM_0, (gpio_num_t)pins.sda, (gpio_num_t)pins.scl);
#endif
            }
        case I2CPins::Backend::ExI2C:
            // NanoC6 / NanoH2: M5Unified manages Ex_I2C -> borrow
            return i2cClass(units, unit, M5.Ex_I2C);
    }
    return false;
}

//! @brief Add a unit on GPIO, preferring PortB and falling back to PortA (ESP-IDF native)
inline bool addGPIO(UnitUnified& units, Component& unit, const GpioRole role = GpioRole::Both)
{
    auto p = gpioPins(role);
    if (role == GpioRole::OutOnly) p.rx = -1;
    if (role == GpioRole::InOnly) p.tx = -1;
    M5_LIB_LOGI("wiring(ESP-IDF): GPIO rx=%d tx=%d role=%d fallback_a=%d", (int)p.rx, (int)p.tx, (int)role,
                (int)p.fallback_a);
    return units.add(unit, p.rx, p.tx);
}

//! @brief Add a unit on UART, preferring PortC and falling back to PortA (ESP-IDF native)
inline bool addUART(UnitUnified& units, Component& unit, const uint32_t baud = 115200,
                    const UartConfig config = UartConfig::Default)
{
    const auto pins = uartPins();
    M5_LIB_LOGI("wiring(ESP-IDF): UART rx=%d tx=%d baud=%u fallback_a=%d", (int)pins.rx, (int)pins.tx, (unsigned)baud,
                (int)pins.fallback_a);
    auto port = uartPortHandle(defaultUartPort(), (gpio_num_t)pins.rx, (gpio_num_t)pins.tx, baud, config);
    if (port == UART_NUM_MAX) return false;
    return units.add(unit, port);
}

//! @brief Add a unit on the board's shared SD/SPI bus (ESP-IDF native). CS is taken from unit.address().
inline bool addSPI(UnitUnified& units, Component& unit, const uint32_t clock_hz, const uint8_t mode = 0,
                   const uint8_t bit_order = 0)
{
    const auto pins = spiPins();
    M5_LIB_LOGI("wiring(ESP-IDF): addSPI sclk=%d miso=%d mosi=%d clock=%u mode=%u bit_order=%u", (int)pins.sclk,
                (int)pins.miso, (int)pins.mosi, (unsigned)clock_hz, (unsigned)mode, (unsigned)bit_order);
    auto dev = spiDeviceHandle(SPI2_HOST, (gpio_num_t)pins.mosi, (gpio_num_t)pins.miso, (gpio_num_t)pins.sclk, clock_hz,
                               mode, bit_order);
    if (!dev) return false;
    return units.add(unit, dev);  // cs omitted -> Component::assign uses address() as CS
}

//! @brief Add a unit on the board's Hat I2C header (NessoN1 uses Wire1, others Wire) (ESP-IDF native)
inline bool addHatI2C(UnitUnified& units, Component& unit, const uint32_t clock = 400000)
{
    const auto p = hatI2CPins();
    if (p.sda < 0 || p.scl < 0) {
        M5_LIB_LOGE("wiring: Hat I2C unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
    i2c_port_t port;
    if (p.useWire1) {
#if SOC_HP_I2C_NUM >= 2
        port = I2C_NUM_1;  // HP I2C #1 (ESP32 / S2 / S3 / P4 / H2 etc.)
#elif SOC_LP_I2C_SUPPORTED
        port = LP_I2C_NUM_0;  // C6: 2nd I2C is LP (= Arduino Wire1)
#else
        M5_LIB_LOGE("wiring: addHatI2C needs a 2nd I2C port, none available board=0x%02x", (int)M5.getBoard());
        return false;
#endif
    } else {
        port = I2C_NUM_0;
    }
    M5_LIB_LOGI("wiring(ESP-IDF): addHatI2C port=%d sda=%d scl=%d clock=%u", (int)port, (int)p.sda, (int)p.scl,
                (unsigned)clock);
#if __has_include(<driver/i2c_master.h>)
    auto bus = i2cBusHandle(port, (gpio_num_t)p.sda, (gpio_num_t)p.scl, clock);
    if (!bus) return false;
    return units.add(unit, bus);
#else  // legacy driver (IDF 5.0 / 5.1)
    if (!detail::ensureI2CLegacyDriver(port, (gpio_num_t)p.sda, (gpio_num_t)p.scl, clock)) return false;
    return units.add(unit, port, (gpio_num_t)p.sda, (gpio_num_t)p.scl);
#endif
}

//! @brief Add a unit on the board's Hat GPIO header (ESP-IDF native)
inline bool addHatGPIO(UnitUnified& units, Component& unit, const GpioRole role = GpioRole::Both)
{
    auto p = hatGPIOPins();
    if (p.rx < 0 || p.tx < 0) {
        M5_LIB_LOGE("wiring: Hat GPIO unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
    if (role == GpioRole::OutOnly) p.rx = -1;
    if (role == GpioRole::InOnly) p.tx = -1;
    M5_LIB_LOGI("wiring(ESP-IDF): addHatGPIO board=0x%02x rx=%d tx=%d role=%d", (int)M5.getBoard(), (int)p.rx,
                (int)p.tx, (int)role);
    return units.add(unit, p.rx, p.tx);
}

//! @brief Add a unit on the board's Hat UART header (ESP-IDF native)
inline bool addHatUART(UnitUnified& units, Component& unit, const uint32_t baud = 115200,
                       const UartConfig config = UartConfig::Default)
{
    const auto p = hatUARTPins();
    if (p.rx < 0 || p.tx < 0) {
        M5_LIB_LOGE("wiring: Hat UART unsupported board=0x%02x", (int)M5.getBoard());
        return false;
    }
    M5_LIB_LOGI("wiring(ESP-IDF): addHatUART board=0x%02x rx=%d tx=%d baud=%u", (int)M5.getBoard(), (int)p.rx,
                (int)p.tx, (unsigned)baud);
    auto port = uartPortHandle(defaultUartPort(), (gpio_num_t)p.rx, (gpio_num_t)p.tx, baud, config);
    if (port == UART_NUM_MAX) return false;
    return units.add(unit, port);
}
#endif  // __M5UNIFIED_HPP__

#endif  // !ARDUINO
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
