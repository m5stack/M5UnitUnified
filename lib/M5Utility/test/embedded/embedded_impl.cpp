/*
  Replace to Arduino functions

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include <M5Utility.hpp>
#include <Arduino.h>

// Replace to Arduio functions
namespace m5 {
namespace utility {
unsigned long millis() {
    return ::millis();
}

unsigned long micros() {
    return ::micros();
}

void delay(const unsigned long ms) {
    ::delay(ms);
}

void delayMicroseconds(const unsigned int us) {
    ::delayMicroseconds(us);
}

}  // namespace utility
}  // namespace m5
