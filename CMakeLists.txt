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
include($ENV{IDF_PATH}/tools/cmake/version.cmake)

file(GLOB_RECURSE SRCS "src/*.cpp")
list(FILTER SRCS EXCLUDE REGEX "/googletest/")

set(public_requires driver M5HAL)
set(private_requires esp_adc esp_timer M5Utility)

# Keep IDF 5.x on the driver meta-component. IDF v6 no longer exposes all split
# driver include paths transitively, so add only the v6-specific component deps.
if("${IDF_VERSION_MAJOR}" VERSION_GREATER_EQUAL "6")
    list(APPEND public_requires esp_driver_gpio esp_driver_i2c esp_driver_rmt esp_driver_spi esp_driver_uart esp_hw_support)
    list(APPEND private_requires esp_driver_ledc esp_ringbuf)
endif()

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "src"
    REQUIRES ${public_requires}
    PRIV_REQUIRES ${private_requires}
)

# Optionally link M5Unified only when it is already part of the build (e.g. a M5Unified-based
# app or example). This puts M5GFX's <utility/I2C_Class.hpp> on adapter_i2c.cpp's include path,
# so the real m5::I2C_Class adapter is compiled instead of the stub. Pure ESP-IDF builds without
# M5Unified are unaffected (the component is simply skipped). Available since IDF v5.0.
# The component name depends on where M5Unified comes from, so list every form:
# m5stack__m5unified (ESP Component Registry, whose component name is lower case),
# m5stack__M5Unified (git dependency keyed as m5stack/M5Unified) and M5Unified (local components/).
# A name that does not match is skipped silently, which builds the I2C_Class stub instead.
idf_component_optional_requires(PRIVATE m5stack__m5unified m5stack__M5Unified M5Unified)
