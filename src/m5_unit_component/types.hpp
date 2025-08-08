/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file types.hpp
  @brief Type and enumerator definitions
*/
#ifndef M5_UNIT_COMPONENT_TYPES_HPP
#define M5_UNIT_COMPONENT_TYPES_HPP

#include <cstdint>
#include <type_traits>
#include "identify_functions.hpp"
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
#include <driver/rmt_types.h>
#else
#include <soc/rmt_struct.h>
#endif
#include <driver/gpio.h>

namespace m5 {
namespace unit {
/*!
  @namespace types
  @brief Type and enumerator definitions
 */
namespace types {

/*!
  @enum category_t
  @brief Unit category (used for static class determination)
 */
enum class category_t {
    None,
    UnitLED,  //!< Derived from UnitLED
};

using uid_t          = uint32_t;       //!< @brief Component unique identifier
using attr_t         = uint32_t;       //!< @brief Component attribute bits
using elapsed_time_t = unsigned long;  //!< @brief Elapsed time unit (ms)

namespace attribute {
///@name Attribute
///@{
constexpr attr_t AccessI2C  = 0x00000001;  //!< I2C Accessible Unit
constexpr attr_t AccessGPIO = 0x00000002;  //!< GPIO Accessible Unit
constexpr attr_t AccessUART = 0x00000004;  //!< UART Accessible Unit
///@}
}  // namespace attribute

}  // namespace types

namespace gpio {
/*!
  @enum Mode
  @brief Pin mode
 */
enum class Mode : uint8_t {
    Input,
    Output,
    Pullup,
    InputPullup,
    Pulldown,
    InputPulldown,
    OpenDrain,
    OutputOpenDrain,
    Analog,
    // RMT access
    RmtRX = 0x80,
    RmtTX,
    RmtRXTX,
};

/*!
  @struct m5::unit::gpio::adapter_config_t
  @brief Unified configuration for RMT v1/v2
*/
struct adapter_config_t {
    //! @brief common
    struct config_t {
        uint32_t tick_ns{};     ///< RMT tick resolution (in nanoseconds)
        gpio_num_t gpio_num{};  ///< GPIO pin number
        uint8_t mem_blocks{};   ///< RMT memory block count (v1) or symbol blocks (v2)
        bool invert_signal{};   ///< Invert input/output logic level
        bool with_dma{};        ///< Use DMA (v2 only)
    };

    //! @brief For TX
    struct tx_config_t : config_t {
        uint16_t loop_count{1};      ///< Number of times to loop (v2 only, ignored if loop_enabled=false)
        bool idle_output_enabled{};  ///< Enable output when idle (idle_level applies)
        bool idle_level_high{};      ///< Idle level HIGH if true, LOW otherwise
        bool loop_enabled{};         ///< Loop mode (TX repeats automatically)
        //        bool carrier_enabled{};      ///< Enable carrier modulation (not commonly used)
    };
    //! @brief For RX
    struct rx_config_t : config_t {
        uint16_t ring_buffer_size{};        ///< Ring buffer size for RX (bytes, v1 only)
        uint16_t filter_ticks_threshold{};  ///< Filter: min valid pulse duration (in ticks)
        uint16_t idle_ticks_threshold{};    ///< RX idle threshold (in ticks for v1, in us for v2)
        bool filter_enabled{};              ///< Enable input signal filter
        //        bool eof_flag{};                    ///< Use RX EOF detection via timeout (v2 feature)
    };

    Mode mode{};       ///< Operating mode (RmtRX.RmtTx,RmtRXTX)
    rx_config_t rx{};  ///< For RX
    tx_config_t tx{};  ///< For TX
};

// Alias
#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
using m5_rmt_item_t = rmt_symbol_word_t;  //!< Alias for RMT item
#else
using m5_rmt_item_t = rmt_item32_t;  //!< Alias for RMT item
#endif

}  // namespace gpio

}  // namespace unit
}  // namespace m5
#endif
