/*!
  @file utility.hpp
  @brief Utilities for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UNIT_UNIFIED_UTILITY_HPP
#define M5_UNIT_UNIFIED_UTILITY_HPP

#include <m5_utility/types.hpp>
#include <m5_utility/crc.hpp>

namespace m5 {
namespace unit {
namespace utility {

/*!
  @class DataWithCRC
  @brief Accessors for big_uint16_t and crc8 pairs of sequences
  @datail Accessors for processing typical read data<br>
  @detail [uint8_t, uint8_t, uint8_t] [0][1]:data(big endian) [2]:CRC for data
 */
class DataWithCRC {
   public:
    /*!
      @brief Constructor
      @param[in] ptr Data pointer
      @param[in] num Number of pairs (Not data length)
      @warning Be careful not to access out of range
     */
    DataWithCRC(const uint8_t* ptr, const size_t num) : _data(ptr), _num(num) {
        assert(ptr);
        assert(num);
    }

    /*!
      @brief Is data valid?
      @param[in] n Subscript indicating the number of pairs
      @return True if valid
     */
    bool valid(const size_t n) const {
        return n < (_num) ? crc8.get(_data + n * 3, 2) == _data[n * 3 + 2]
                          : false;
    }
    /*!
      @brief Gets the value_
      @param[in] n Subscript indicating the number of pairs
    */
    uint16_t value(const size_t n) const {
        assert(n < _num);
        return m5::types::big_uint16_t(_data[n * 3], _data[n * 3 + 1]).get();
    }

   private:
    const uint8_t* _data{};
    const size_t _num{};

    static m5::utility::CRC8_Maxim crc8;
};

}  // namespace utility
}  // namespace unit
}  // namespace m5

#endif
