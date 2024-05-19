#ifndef M5_HAL_PLATFORM_CHECKER_HPP
#define M5_HAL_PLATFORM_CHECKER_HPP

#define M5HAL_PLATFORM_NUMBER_UNKNOWN              0
#define M5HAL_PLATFORM_NUMBER_WINDOWS             10
#define M5HAL_PLATFORM_NUMBER_MACOS               20
#define M5HAL_PLATFORM_NUMBER_LINUX               30

#define M5HAL_PLATFORM_NUMBER_SDL_MAX             99

#define M5HAL_PLATFORM_NUMBER_AVR                200
#define M5HAL_PLATFORM_NUMBER_ESP8266            300
#define M5HAL_PLATFORM_NUMBER_ESP32_UNKNOWN      310
#define M5HAL_PLATFORM_NUMBER_ESP32_1st          311
#define M5HAL_PLATFORM_NUMBER_ESP32_S2           312
#define M5HAL_PLATFORM_NUMBER_ESP32_S3           313
#define M5HAL_PLATFORM_NUMBER_ESP32_C3           314
#define M5HAL_PLATFORM_NUMBER_ESP32_C6           315
#define M5HAL_PLATFORM_NUMBER_RP2040             400
#define M5HAL_PLATFORM_NUMBER_SAMD21             500
#define M5HAL_PLATFORM_NUMBER_SAMD51             510
#define M5HAL_PLATFORM_NUMBER_SPRESENSE          600
#define M5HAL_PLATFORM_NUMBER_STM32              700

#define XSTR(x) STR(x)
#define STR(x) #x


#if defined ( ESP_PLATFORM )
    #if __has_include (<sdkconfig.h>)
    #include <sdkconfig.h>
    #endif
    #if defined ( CONFIG_IDF_TARGET )
        #if defined ( CONFIG_IDF_TARGET_ESP32C3 )
            #define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_ESP32C3
            #define M5HAL_TARGET_PLATFORM_PATH m5_hal/platforms/espressif/esp32c3
        #elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
            #define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_ESP32S3
            #define M5HAL_TARGET_PLATFORM_PATH m5_hal/platforms/espressif/esp32s3
        #elif defined ( CONFIG_IDF_TARGET_ESP32 )
            #define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_ESP32_1st
            #define M5HAL_TARGET_PLATFORM_PATH m5_hal/platforms/espressif/esp32

        #elif defined ( CONFIG_IDF_TARGET_ESP32C6 )
            #define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_ESP32_C6
            #define M5HAL_TARGET_PLATFORM_PATH m5_hal/platforms/espressif/esp32c6
        #else
            #define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_ESP32_UNKNOWN
            #define M5HAL_TARGET_PLATFORM_PATH m5_hal/platforms/espressif/esp32_unknown
        #endif
    #endif
#else
#endif

#undef XSTR
#undef STR

            
#if !defined ( M5HAL_TARGET_PLATFORM_NUMBER )
#define M5HAL_TARGET_PLATFORM_NUMBER M5HAL_PLATFORM_NUMBER_UNKNOWN
#endif

#endif
