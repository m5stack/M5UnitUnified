#ifndef M5_HAL_PLATFORMS_ESPRESSIF_ESP32_HEADER_HPP
#define M5_HAL_PLATFORMS_ESPRESSIF_ESP32_HEADER_HPP

#include "../../../interface/gpio.hpp"
#include "../../../bus/bus.hpp"

namespace m5 {
namespace hal {
namespace platforms {

// namespace esp32 {}
// namespace current_platform {
//     using namespace esp32;
// }

namespace esp32 {
namespace types {

enum class PeripheralType : uint8_t {
  none = 0,
  i2c0,
  i2c1,
  spi2,
  spi3,
};
using periph_t = PeripheralType;

} // namespace types

namespace gpio {
interface::gpio::GPIO* getGPIO(void);
} // namespace gpio

} // namespace esp32
} // namespace platforms
} // namespace hal
} // namespace m5

#endif
