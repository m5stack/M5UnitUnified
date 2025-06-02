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

using uid_t          = uint32_t;       //!< @brief Component unique identifier
using attr_t         = uint32_t;       //!< @brief Component attribute bits
using elapsed_time_t = unsigned long;  //!< @brief Elapsed time unit (ms)

namespace attribute {
///@name Attribute
///@{
constexpr attr_t AccessI2C  = 0x00000001;  //!< I2C Accessible Unit
constexpr attr_t AccessGPIO = 0x00000002;  //!< GPIO Accessible Unit
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
  @brief Common pinMode, RMT v1 and v2 settings
 */
struct adapter_config_t {
    struct config_t {
        gpio_num_t gpio_num{};
        uint32_t tick_ns{};
        uint8_t mem_blocks{};
        bool idle_output{};
        bool idle_level_high{};
        bool with_dma{};
        bool loop_enabled{};
    };
    Mode mode{};    // Mode
    config_t rx{};  // For RMT
    config_t tx{};  // For RMT
};

#if defined(M5_UNIT_UNIFIED_USING_RMT_V2)
using m5_rmt_item_t = rmt_symbol_word_t;
#else
using m5_rmt_item_t = rmt_item32_t;
#endif

}  // namespace gpio

}  // namespace unit
}  // namespace m5
#endif
