
#ifndef M5_HAL_ERROR_HPP
#define M5_HAL_ERROR_HPP

#include <stdint.h>

namespace m5 {
namespace hal {
namespace error {

enum class ErrorType : int8_t {
  ASYNC_RUNNING = 1,
  OK = 0,
  UNKNOWN_ERROR = -1,
  TIMEOUT_ERROR = -2,
  INVALID_ARGUMENT = -3,
  NOT_IMPLEMENTED = -4,
  I2C_BUS_ERROR = -5,
  I2C_NO_ACK = -6,
// @ TODO エラーコード細分化する
};
using error_t = ErrorType;

constexpr bool isError(const error_t e) {
  return e < error_t::OK;
}
constexpr bool isOk(const error_t e) {
  return e == error_t::OK;
}

} // namespace error
} // namespace hal
} // namespace m5

#endif
