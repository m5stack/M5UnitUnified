#ifndef M5_HAL_PLATFORMS_ESPRESSIF_ESP32S3_HEADER_HPP
#define M5_HAL_PLATFORMS_ESPRESSIF_ESP32S3_HEADER_HPP

#include "../../../interface/gpio.hpp"
#include "../../../interface/bus.hpp"

namespace m5 {
namespace hal {
namespace platforms {
namespace esp32 {
namespace gpio {
interface::gpio::GPIO* getGPIO(void);
} // namespace gpio

namespace bus {
namespace i2c { 


} // namespace i2c
} // namespace bus
} // namespace esp32
} // namespace platforms
} // namespace hal
} // namespace m5

#endif
