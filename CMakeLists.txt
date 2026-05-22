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
# REQUIRES = public deps (exposed through src/ public headers); PRIV_REQUIRES = used only in .cpp.
# Use the `driver` meta-component (exists in all IDF 5.x and publicly aggregates esp_driver_*),
# NOT the split esp_driver_* components which only exist from IDF 5.3 and would break >=5.0.
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "src"
    REQUIRES driver M5HAL
    PRIV_REQUIRES esp_adc esp_timer M5Utility
)
