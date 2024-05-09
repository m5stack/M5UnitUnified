/*!
  @file library_log.cpp
  @brief Logging for libraries

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#include "library_log.hpp"
#include <cstdio>
#include <cstdarg>

namespace {
using clock                      = std::chrono::steady_clock;
//using clock                      = std::chrono::high_resolution_clock;
const clock::time_point start_at = clock::now();
}  // namespace

namespace m5 {
namespace utility {
namespace log {

void log_printf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    putchar('\n');
#if !defined(ARDUINO)
    fflush(stdout);
#endif
}

elapsed_time_t elapsedTime() {
    return std::chrono::duration_cast<elapsed_time_t>(clock::now() - start_at);
}

}  // namespace log
}  // namespace utility
}  // namespace m5
