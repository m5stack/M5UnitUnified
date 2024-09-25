/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @brief Configrate time by NTP
  @file config_time.cpp
*/
#ifndef CONFIG_TIME_HPP
#define CONFIG_TIME_HPP

bool isEnabledRTC();
void setTimezone(const char* posix_tz = nullptr);
bool configTime(const char* ssid = nullptr, const char* password = nullptr);

#endif
