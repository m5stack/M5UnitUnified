/*!
  @file string.cpp
  @brief Utilities for string

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/

#include "string.hpp"
#include <cstdarg>
#include <algorithm>

namespace m5 {
namespace utility {

std::string formatString(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t sz = vsnprintf(nullptr, 0U, fmt, args);  // calculate length
    va_end(args);

    char buf[sz + 1];
    va_start(args, fmt);  // Reinitiaize args (args cannot reuse because
                          // indefinite value after vsnprintf)
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    // String don't has constructor(const char*, const size_t);
    buf[sz] = '\0';
    return std::string(buf);
}

std::string& trimRight(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](std::string::value_type& ch) {
                return !std::isspace(ch);
            }));
    return s;
}

std::string& trimLeft(std::string& s) {
    s.erase(std::find_if(
                s.rbegin(), s.rend(),
                [](std::string::value_type& ch) { return !std::isspace(ch); })
                .base(),
            s.end());
    return s;
}

std::string& trim(std::string& s)
{
    return trimRight(trimLeft(s));
}


}  // namespace utility
}  // namespace m5
