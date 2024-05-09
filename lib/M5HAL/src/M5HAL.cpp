
#include "M5HAL.hpp"
#include "m5_hal/bus/bus.inl"
#include "m5_hal/bus/i2c.inl"
#include "m5_hal/bus/spi.inl"

#define M5HAL_STATIC_MACRO_PATH_IMPL M5HAL_STATIC_MACRO_CONCAT(M5HAL_TARGET_PLATFORM_PATH, impl.inl)

#if M5HAL_TARGET_PLATFORM_NUMBER != 0
    #include M5HAL_STATIC_MACRO_PATH_IMPL
#endif

// Arduino環境の場合のみ、Arduino用の実装を用意
#if M5HAL_FRAMEWORK_HAS_ARDUINO
    #include "./m5_hal/frameworks/arduino/impl.inl"
#endif

