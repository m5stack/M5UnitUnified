/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @brief Configrate time
  @file config_time.cpp
*/
#include "config_time.hpp"
#include <M5Unified.h>
#include <M5Utility.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <random>
#include <algorithm>

namespace {
// NTP server URI
constexpr char ntp0[]     = "ntp.nict.jp";
constexpr char ntp1[]     = "ntp.jst.mfeed.ad.jp";
constexpr char ntp2[]     = "time.cloudflare.com";
const char* ntpURLTable[] = {ntp0, ntp1, ntp2};

constexpr char defaultPosixTZ[] = "JST-9";  // Asia/Tokyo

auto rng = std::default_random_engine{};
}  // namespace

bool isEnabledRTC()
{
    // Check RTC if exists
    if (M5.Rtc.isEnabled()) {
        auto dt = M5.Rtc.getDateTime();  // GMT
        if (dt.date.year > 2016) {
            M5_LOGV("RTC time already set. (GMT) %04d/%02d/%2d %02d:%02d:%02d", dt.date.year, dt.date.month,
                    dt.date.date, dt.time.hours, dt.time.minutes, dt.time.seconds);
            return true;
        }
        M5_LOGW("RTC is not set to the correct time");
    }
    return false;
}

void setTimezone(const char* posix_tz)
{
    setenv("TZ", posix_tz ? posix_tz : defaultPosixTZ, 1);
    tzset();
}

bool configTime(const char* posix_tz, const char* ssid, const char* password)
{
    M5_LOGI("Configrate time");

    // WiFi connect
    if (ssid && password) {
        WiFi.begin(ssid, password);
    } else {
        // Connect to credential in Hardware. (ESP32 saves the last WiFi connection)
        WiFi.begin();
    }
    int32_t retry{10};
    while (WiFi.status() != WL_CONNECTED && --retry >= 0) {
        M5_LOGI(".");
        m5::utility::delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED) {
        M5_LOGE("Failed to connect WiFi");
        return false;
    }

    std::shuffle(std::begin(ntpURLTable), std::end(ntpURLTable), rng);
    configTzTime(posix_tz ? posix_tz : defaultPosixTZ, ntpURLTable[0], ntpURLTable[1], ntpURLTable[2]);

    // Waiting for time synchronization
    retry = 10;
    sntp_sync_status_t st{};
    while (((st = sntp_get_sync_status()) == SNTP_SYNC_STATUS_RESET) && --retry >= 0) {
        M5_LOGI("Time synchronization in progress");
        m5::utility::delay(1000);
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    std::tm discard{};
    if ((st != SNTP_SYNC_STATUS_COMPLETED) || !getLocalTime(&discard, 10 * 1000 /* timeout */)) {
        M5_LOGE("Failed to sync time");
        return false;
    }

    // Set RTC if exists
    if (M5.Rtc.isEnabled()) {
        time_t t = time(nullptr) + 1;
        while (t > time(nullptr)) {
            /* Nop */
        }
        M5.Rtc.setDateTime(std::gmtime(&t));
    }
    return true;
}
