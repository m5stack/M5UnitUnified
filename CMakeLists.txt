# ESP-IDF component manifest for M5UnitUnified.
# Upstream: https://github.com/m5stack/M5UnitUnified
#
# Notes
# - All Arduino-specific code paths (TwoWire / HardwareSerial / Wire.h /
#   HardwareSerial.h) are guarded with `#if defined(ARDUINO)` upstream, so
#   under pure ESP-IDF those branches are skipped. The ESPIDFMasterBus path
#   (i2c_master_bus_handle_t) is the one used in non-Arduino builds.
# - googletest helpers (src/googletest/*) are EXCLUDED — they pull in
#   <M5Unified.h> for test scaffolding only and are not needed at runtime.
file(GLOB_RECURSE SRCS "src/*.cpp")
list(FILTER SRCS EXCLUDE REGEX "/googletest/")
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "src"
    REQUIRES driver esp_driver_uart esp_driver_gpio esp_driver_spi esp_hw_support esp_adc esp_timer M5HAL M5Utility freertos
)
