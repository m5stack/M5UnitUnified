/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file utility.hpp
  @brief Utilities for M5UnitComponent
*/
#ifndef M5_UNIT_COMPONENT_UTILITY_HPP
#define M5_UNIT_COMPONENT_UTILITY_HPP

#include <m5_utility/types.hpp>
#include <m5_utility/crc.hpp>
#include <cassert>

namespace m5 {
namespace unit {
/*!
  @namespace utility
  @brief Utilities for M5UnitComponent
 */
namespace utility {

///@cond
class WithCRC {
   public:
    WithCRC() {
    }

   protected:
    //static m5::utility::CRC8_Checksum crc8;
};
///@endcond

/*!
  @class ReadDataWithCRC16
  @brief Accessors for big_uint16_t and crc8 pairs of sequences
  @details Accessors for processing typical read data<br>
  @details [uint8_t, uint8_t, uint8_t] [0][1]:data(big endian) [2]:CRC for data
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
        : WithCRC(), _data(ptr), _num(num) {
        assert(ptr);
        assert(num);
    }

    /*!
      @brief Is data valid?
      @param[in] n Subscript indicating the number of pairs
      @return True if valid
     */
    bool valid(const size_t n) const {
        m5::utility::CRC8_Checksum crc8{};
        return n < (_num) ? crc8.update(_data + n * 3, 2) == _data[n * 3 + 2]
                          : false;
    }
    /*!
      @brief Gets the value
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
    explicit WriteDataWithCRC16(const uint16_t v = 0) : WithCRC() {
#if 0
        new (_buf) m5::types::big_uint16_t(v);  // Placement new
#else
        m5::types::big_uint16_t tmp(v);
        _buf[0] = tmp.u8[0];
        _buf[1] = tmp.u8[1];
#endif
        m5::utility::CRC8_Checksum crc8{};
        _buf[2] = crc8.update(_buf, 2);
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

#if 0
/*!
  @struct BigInt16ArrayWithCRC
  @tparam T Type of array elements
  @tparam sz 
  @tparam CRC Class used for CRC calculation
  ~~~
  Array image
  [0][1] bigint16_t data0 [2] CRC for data0 index :0
  [3][4] bigint16_t data1 [5] CRC for data0 index: 1
  ...
  [n][n+1] bigint16_t datan [n+2] CRC for datan index:n
  ~~~
*/
template <size_t sz, class CRC = m5::utility::CRC8_Checksum>
class BigInt16ArrayWithCRC {
    static_assert(sz % 3 == 0, "Invalid size");

   public:
    type_t = m5::types::U16 <

             inline ArrayWithCRC() {
    }
    inline ArrayWithCRC(const uint8_t* p, const size_t len) {
        assert(len == _array.size() h && "Invalid length");
        memcpy(_array.data(), p, len);
    }

    /*!
      @brief Is the CRC of the data at the specified index correct?
      @param idx Index of data(See also Array image)
      @return True if correct
     */
    bool validCRC(const uint8_t idx) {
        CRC crc;
        return idx * 3 < _array.size()
                   ? crc.update(
                         big_uint16_t(_array[idx * 3], _array[idx * 3 + 1])
                             .data(),
                         2U) == _array[idx * 3 + 2]
                   : false;
    }
    /*!
      @brief Calculate and store the CRC of the specified index
      @param idx Index of data(See also Array image)
      @return True if successful
    */
    bool storeCRC(const uint8_t idx) {
        CRC crc;
        if (idx * 3 < _array.size()) {
            _array[idx * 3 + 2] = crc.update(
                big_uint16_t(_array[idx * 3], _array[idx * 3 + 1]).data(), 2U);
            return true;
        }
        return false;
    }

    //!@brief Gets the data pointer
    inline const uint8_t* data() const {
        return _array.data();
    }
    //!@brief Gets the data pointer
    inline uint8_t* data() {
        return _array.data();
    }
    //! @gbrief Gets the size of the array(include CRC)
    inline size_t size() const {
        return sz;
    }

   private:
    std::array<uint8_t, sz> _array{};
};
#endif

}  // namespace utility
}  // namespace unit
}  // namespace m5

#endif
