/*!
  @file compatibility_feature.hpp
  @brief Maintain compatibility with Arduino API, etc.

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT

*/
#ifndef M5_UTILITY_COMPATIBILITY_FEATURE_HPP
#define M5_UTILITY_COMPATIBILITY_FEATURE_HPP

namespace m5 {
namespace utility {

///@name Arduino API
///@{
/*!
  @brief Returns the number of milliseconds passed since the Arduino board began running the current program
 */
__attribute__((weak)) unsigned long millis();
/*!
  @brief Returns the number of microseconds since the Arduino board began running the current program
*/
__attribute__((weak)) unsigned long micros();
/*!
  @brief Pauses the program for the amount of time (in milliseconds) specified as parameter.
  @warning Accuracy varies depending on the environment.
*/
__attribute__((weak)) void delay(const unsigned long ms);
/*!
  @brief Pauses the program for the amount of time (in microseconds) specified by the parameter. 
  @warning Accuracy varies depending on the environment.
*/
__attribute__((weak)) void delayMicroseconds(const unsigned int us);
///@}

}  // namespace utility
}  // namespace m5
#endif
