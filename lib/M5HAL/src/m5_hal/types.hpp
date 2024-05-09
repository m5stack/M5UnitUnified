
#ifndef M5_HAL_TYPES_HPP
#define M5_HAL_TYPES_HPP

#include <stdint.h>
#include <stddef.h>
// #include <M5Utility.hpp>

namespace m5 {

namespace hal {

namespace types {

enum class PeripheralType : uint8_t;
using periph_t = PeripheralType;

// GPIOピンの通し番号型
typedef int16_t gpio_number_t;
// typedef m5::utility::BitSegment<3, int16_t> gpio_number_t;
// class gpio_number_t : public m5::utility::BitSegment<3, int16_t> {
//   public:
//     inline constexpr gpio_number_t() = default;
//     inline constexpr gpio_number_t(const base_type v) : BitSegment(v) {
//     }
//     inline constexpr int16_t port(void) const { return upper(); }
//     inline constexpr int16_t pin(void) const { return lower(); }
//     inline void port(int16_t v) { upper(v); }
//     inline void pin(int16_t v) { lower(v); }
// };
/*
    マイナスの値は未設定や無効値として扱う
※ : ポートに所属しているピンの数が3bit (8本) の場合
gpio_number_t の下位 3bit がピン番号、上位 12bit がポート番号となる

※ : ポートに所属しているピンの数が5bit (32本) の場合
gpio_number_t の下位 5bit がピン番号、上位 10bit がポート番号となる
*/

enum class GpioMode : uint8_t {
  Input = 0,
  Output,
  Output_OpenDrain,
};
using gpio_mode_t = GpioMode;


enum class BusType : uint8_t {
  UNKNOWN = 0,
  I2C,
  SPI,
  I2S,
  UART,

  PWM,
  GPIO,
  ADC,
  DAC,
};
using bus_type_t = BusType;

} // namespace types
} // namespace hal
} // namespace m5

#endif
