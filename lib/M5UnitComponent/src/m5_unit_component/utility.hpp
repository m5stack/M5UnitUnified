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
#include <cassert>

namespace m5 {
namespace unit {
namespace utility {

///@cond
class WithCRC {
   protected:
    static m5::utility::CRC8_Maxim crc8;
};
///@endcond

/*!
  @class ReadDataWithCRC16
  @brief Accessors for big_uint16_t and crc8 pairs of sequences
  @detail Accessors for processing typical read data<br>
  @detail [uint8_t, uint8_t, uint8_t] [0][1]:data(big endian) [2]:CRC for data
 */
class ReadDataWithCRC16 : public WithCRC {
   public:
    /*!
      @brief Assign pointer
      @param[in] ptr Data pointer
      @param[in] num Number of pairs (Not data length)
      @warning Be careful not to access out of range
     */
    ReadDataWithCRC16(const uint8_t* ptr, const size_t num)
        : _data(ptr), _num(num) {
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
};

/*!
  @class WriteDataWithCRC16
  @brief Make a array of uint16_t(big endian) and CRC8
 */
class WriteDataWithCRC16 : public WithCRC {
   public:
    /*!
      @brief Set value and calculate CRC
     */
    explicit WriteDataWithCRC16(const uint16_t v = 0) {
        new (_buf) m5::types::big_uint16_t(v);  // placement new
        _buf[2] = crc8.get(_buf, 2);
    }
    //! @brief Gets the const pointer
    const uint8_t* data() const {
        return _buf;
    }
    //! @brief Gets size in uint8_t units.
    size_t size() const {
        return sizeof(_buf);
    }

   private:
    uint8_t _buf[3]{};
};

}  // namespace utility
}  // namespace unit
}  // namespace m5

#endif
