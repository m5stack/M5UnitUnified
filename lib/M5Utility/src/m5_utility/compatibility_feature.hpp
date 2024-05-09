/*!
  @file compatibility_feature.hpp
  @brief Maintain compatibility with Arduino API, etc.

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
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
unsigned long millis();
/*!
  @brief Returns the number of microseconds since the Arduino board began running the current program
*/
unsigned long micros();
/*!
  @brief Pauses the program for the amount of time (in milliseconds) specified as parameter.
  @warning Accuracy varies depending on the environment.
*/
void delay(const unsigned long ms);
/*!
  @brief Pauses the program for the amount of time (in microseconds) specified by the parameter. 
  @warning Accuracy varies depending on the environment.
*/
void delayMicroseconds(const unsigned int us);
///@}

}  // namespace utility
}  // namespace m5
#endif
