/*!
  @file library_log.cpp
  @brief Logging for libraries

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "library_log.hpp"
#include <cstdio>
#include <cstdarg>
#include <inttypes.h>
#include <algorithm>
#include <cctype>

namespace {
using clock = std::chrono::steady_clock;
// using clock                      = std::chrono::high_resolution_clock;
const clock::time_point start_at = clock::now();
}  // namespace

namespace m5 {
namespace utility {
namespace log {

void logPrintf(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
#if !defined(ARDUINO)
    fflush(stdout);
#endif
}

void dump(const void* iaddr, const size_t len, const bool align) {
    constexpr static char hc[] = "0123456789ABCDEF";
    uint8_t* addr              = (uint8_t*)iaddr;
    uint8_t abyte = align ? 0x0F : 0x00;

    uint_fast8_t skip_left{(uint_fast8_t)((uintptr_t)addr & abyte)};

    char hex[128]{};

    printf("DUMP:0x%08" PRIxPTR " %zu bytes\n", (uintptr_t)addr, len);
    
    // First line
    size_t i{}, ia{};
    uint_fast8_t cols = std::min(len - i, (size_t)16 - skip_left);
    uint8_t left      = snprintf(hex, sizeof(hex), "0x%08" PRIxPTR "| ",
                                 (uintptr_t)addr & ~abyte);

    for (uint_fast8_t s = 0; s < skip_left; ++s) {
        hex[left++] = ' ';
        hex[left++] = ' ';
        hex[left++] = ' ';
    }
    for (uint_fast8_t c = 0; c < cols; ++c) {
        left += snprintf(hex + left, 4, "%c%c ", hc[(addr[i] >> 4) & 0x0F],
                         hc[addr[i] & 0x0F]);
        ++i;
    }
    for (uint_fast8_t s = skip_left; s < 16U - cols; ++s) {
        hex[left++] = ' ';
        hex[left++] = ' ';
        hex[left++] = ' ';
    }

    hex[left++] = '|';
    for (uint_fast8_t s = 0; s < skip_left; ++s) {
        hex[left++] = ' ';
    }
    for (uint_fast8_t c = 0; c < cols; ++c) {
        left += snprintf(hex + left, 2, "%c",
                         std::isprint(addr[ia]) ? (char)addr[ia] : '.');
        ++ia;
    }
    puts(hex);

    // Second line~
    while (i < len) {
        cols = std::min(len - i, (size_t)16U);
        left = snprintf(hex, sizeof(hex), "0x%08" PRIxPTR "| ",
                        (uintptr_t)(addr + i) & ~abyte);
        for (uint_fast8_t c = 0; c < cols; ++c) {
            left += snprintf(hex + left, 4, "%c%c ", hc[(addr[i] >> 4) & 0x0F],
                             hc[addr[i] & 0x0F]);
            ++i;
        }
        for (uint_fast8_t s = 0; s < 16U - cols; ++s) {
            hex[left++] = ' ';
            hex[left++] = ' ';
            hex[left++] = ' ';
        }

        hex[left++] = '|';
        for (uint_fast8_t c = 0; c < cols; ++c) {
            left += snprintf(hex + left, 2, "%c",
                             std::isprint(addr[ia]) ? (char)addr[ia] : '.');
            ++ia;
        }
        puts(hex);
    }
}

elapsed_time_t elapsedTime() {
    return std::chrono::duration_cast<elapsed_time_t>(clock::now() - start_at);
}

}  // namespace log
}  // namespace utility
}  // namespace m5
