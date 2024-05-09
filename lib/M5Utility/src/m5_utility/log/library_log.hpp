/*!
  @file library_log.hpp
  @brief Logging for libraries

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_LOG_LIBRARY_LOG_HPP
#define M5_UTILITY_LOG_LIBRARY_LOG_HPP

#include <cstdint>
#include <chrono>

namespace m5 {
namespace utility {
namespace log {

/*!
  @enum log_level_t
  @brief Log output control level
  @details
*/
enum class LogLevel : uint8_t {
    NONE,     //!< No output
    ERROR,    //!< Error
    WARN,     //!< Warning
    INFO,     //!< Information
    DEBUG,    //!< Debug
    VERBOSE,  //!< Verbose
};
using log_level_t = LogLevel;

#if defined(NDEBUG)
constexpr log_level_t logOutputLevel = log_level_t::NONE;
#elif defined(M5_LOG_LEVEL)
constexpr log_level_t logOutputLevel = static_cast<log_level_t>(M5_LOG_LEVEL);
#elif defined(CORE_DEBUG_LEVEL)
constexpr log_level_t logOutputLevel =
    static_cast<log_level_t>(CORE_DEBUG_LEVEL);
#else
/*!
  @var logOutputLevel
  @brief Base value of log level to be output
  @details The value can be specified in the compile options.
  -DM5_LOG_LEVEL=[0..5] or -DCORE_LOG_LEVEL=[0...5]
  default as NONE
  @warning No output if NDEBUG defined
 */
constexpr log_level_t logOutputLevel = log_level_t::NONE;
#endif

/// @cond
// Does the string contain slash?
constexpr bool containss_slash(const char* s) {
    return *s ? (*s == '/' ? true : containss_slash(s + 1)) : false;
}
// Returns the next position after the right-most slash
constexpr const char* after_right_slash(const char* s) {
    return (*s == '/') ? (s + 1) : after_right_slash(s - 1);
}
// Gets the tail of string
constexpr const char* tail(const char* s) {
    return *s ? tail(s + 1) : s;
}
/// @endcond

/*!
  @brief Gets the filename from full pathname
  @warning If the string is too long, the recursion depth may be too deep to
  fail. (If compile time calculation)
 */
constexpr const char* pathToFilename(const char* path) {
    return (path && path[0])
               ? (containss_slash(path) ? after_right_slash(tail(path)) : path)
               : "";
}

//! @brief Output formatted strings
void log_printf(const char* format, ...);

using elapsed_time_t = std::chrono::milliseconds;
// using elapsed_time_t = std::chrono::microseconds;

//! @brief Gets the elapsed time for log
elapsed_time_t elapsedTime();

///@cond
#ifndef M5_UTILITY_LOG_FORMAT
#define M5_UTILITY_LOG_FORMAT(letter, format)             \
    "[%6lld][" #letter "][%s:%u] %s(): " format "\n",     \
        (int64_t)m5::utility::log::elapsedTime().count(), \
        m5::utility::log::pathToFilename(__FILE__), __LINE__, __func__
#endif
///@endcond

/*!
  @def M5_LIB_LOGE
  @brief Output log (level ERROR)
 */
#define M5_LIB_LOGE(format, ...)                                           \
    do {                                                                   \
        if (m5::utility::log::logOutputLevel >=                            \
            m5::utility::log::log_level_t::ERROR) {                        \
            m5::utility::log::log_printf(M5_UTILITY_LOG_FORMAT(E, format), \
                                         ##__VA_ARGS__);                   \
        }                                                                  \
    } while (0)
/*!
  @def M5_LIB_LOGW
  @brief Output log (level WARN)
 */
#define M5_LIB_LOGW(format, ...)                                           \
    do {                                                                   \
        if (m5::utility::log::logOutputLevel >=                            \
            m5::utility::log::log_level_t::WARN) {                         \
            m5::utility::log::log_printf(M5_UTILITY_LOG_FORMAT(W, format), \
                                         ##__VA_ARGS__);                   \
        }                                                                  \
    } while (0)
/*!
  @def M5_LIB_LOGI
  @brief Output log (level INFO)
 */
#define M5_LIB_LOGI(format, ...)                                           \
    do {                                                                   \
        if (m5::utility::log::logOutputLevel >=                            \
            m5::utility::log::log_level_t::INFO) {                         \
            m5::utility::log::log_printf(M5_UTILITY_LOG_FORMAT(I, format), \
                                         ##__VA_ARGS__);                   \
        }                                                                  \
    } while (0)
/*!
  @def M5_LIB_LOGD
  @brief Output log (level DEBUG)
 */
#define M5_LIB_LOGD(format, ...)                                           \
    do {                                                                   \
        if (m5::utility::log::logOutputLevel >=                            \
            m5::utility::log::log_level_t::DEBUG) {                        \
            m5::utility::log::log_printf(M5_UTILITY_LOG_FORMAT(D, format), \
                                         ##__VA_ARGS__);                   \
        }                                                                  \
    } while (0)

/*!
  @def M5_LIB_LOGV
  @brief Output log (level VERBOSE)
 */
#define M5_LIB_LOGV(format, ...)                                           \
    do {                                                                   \
        if (m5::utility::log::logOutputLevel >=                            \
            m5::utility::log::log_level_t::VERBOSE) {                      \
            m5::utility::log::log_printf(M5_UTILITY_LOG_FORMAT(V, format), \
                                         ##__VA_ARGS__);                   \
        }                                                                  \
    } while (0)

}  // namespace log
}  // namespace utility
}  // namespace m5

#endif
