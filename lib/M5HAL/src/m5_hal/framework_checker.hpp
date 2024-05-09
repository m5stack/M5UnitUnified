#ifndef M5_HAL_FRAMEWORK_CHECKER_HPP
#define M5_HAL_FRAMEWORK_CHECKER_HPP


#if defined ( ARDUINO )
    #define M5HAL_FRAMEWORK_HAS_ARDUINO 1
#else
    #define M5HAL_FRAMEWORK_HAS_ARDUINO 0
#endif


#if __has_include(<FreeRTOS.h>) || __has_include(<freertos/FreeRTOS.h>)
    #define M5HAL_FRAMEWORK_HAS_FREERTOS 1
#else
    #define M5HAL_FRAMEWORK_HAS_FREERTOS 0
#endif


#if __has_include(<SDL2/SDL.h>) || __has_include(<SDL.h>)
    #define M5HAL_FRAMEWORK_HAS_SDL 1
#else
    #define M5HAL_FRAMEWORK_HAS_SDL 0
#endif


#endif
