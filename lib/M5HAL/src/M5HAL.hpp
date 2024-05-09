#ifndef M5_HAL_HPP
#define M5_HAL_HPP

#include <M5Utility.hpp>

#include "./m5_hal/platform_checker.hpp"
#include "./m5_hal/framework_checker.hpp"
#include "./m5_hal/bus/i2c.hpp"
#include "./m5_hal/bus/spi.hpp"

#include "./m5_hal/bus/bus.hpp"
#include "./m5_hal/interface/gpio.hpp"

#define M5HAL_STATIC_MACRO_STRING(x) #x
#define M5HAL_STATIC_MACRO_CONCAT(x, y) M5HAL_STATIC_MACRO_STRING(x/y)
#define M5HAL_STATIC_MACRO_PATH_HEADER M5HAL_STATIC_MACRO_CONCAT(M5HAL_TARGET_PLATFORM_PATH, header.hpp)

#if M5HAL_TARGET_PLATFORM_NUMBER != 0
    #include M5HAL_STATIC_MACRO_PATH_HEADER
#endif

#undef M5HAL_STATIC_MACRO_PATH_HEADER

// Arduinoフレームワークがある場合は準備
#if M5HAL_FRAMEWORK_HAS_ARDUINO
    #include "./m5_hal/frameworks/arduino/header.hpp"

// 対象プラットフォームが特定されない場合は Arduino経由の実装を利用
    #if 1 // M5HAL_TARGET_PLATFORM_NUMBER == 0

    namespace m5 {
        namespace hal {
            using namespace frameworks::arduino;
        }
    }

    #endif

#endif

#endif
