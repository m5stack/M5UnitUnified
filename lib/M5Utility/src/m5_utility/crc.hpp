/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file crc.hpp
  @brief Calculate CRC
*/
#ifndef M5_UTILITY_CRC_HPP
#define M5_UTILITY_CRC_HPP

#include <cstdint>
#include <cstddef>
#include "misc.hpp"

namespace m5 {
namespace utility {

/*!
  @class CRC8
  @brief Base class of the CRC8 calculator
 */
class CRC8 {
   public:
    CRC8() = delete;
    /*!
      @param init Initial value
      @param polynormal Generated polynomial
      @param refIn Inverted input?
      @param refOut Inverted output?
      @param xorout Exclusive OR output
     */
    CRC8(const uint8_t init, const uint8_t polynomial, const bool refIn,
         const bool refOut, const uint8_t xorout)
        : _crc{init},
          _init(init),
          _polynomial{polynomial},
          _xorout{xorout},
          _refIn{refIn},
          _refOut{refOut} {
    }
    /*!
      @brief Calculate the CRC of the specified range
      @param data Pointer of the array
      @param len Length of the array
      @return CRC value
     */
    inline uint8_t range(const uint8_t* data, size_t len) {
        auto crc = calculate(data, len, _init, _polynomial, _refIn, _refOut,
                             _xorout, false);
        return finalize(crc, _refOut, _xorout);
    }
    /*!
      @brief Stores the CRC of the specified array using the current internal
      information
      @param data Pointer of the array
      @param len Length of the array
      @return CRC value
      @note Used when you want to calculate the value of the entire divided
      continuous data, such as streaming data
    */
    inline uint8_t update(const uint8_t* data, size_t len) {
        _crc = calculate(data, len, _crc, _polynomial, _refIn, _refOut, _xorout,
                         false);
        return finalize(_crc, _refOut, _xorout);
    }
    /*!
      @brief CRC value at the time of the call
      @return CRC value
     */
    inline uint8_t value() const {
        return finalize(_crc, _refOut, _xorout);
    }
    /*!
      @brief Calculate CRC8
      @param data Pointer of the array
      @param len Length of the array
      @param polynormal Generated polynomial
      @param refIn Inverted input?
      @param refOut Inverted output?
      @param xorout Exclusive OR output
      @param do_finalize Apply processing to output values?(true as defaut)
      @return CRC value
    */
    static uint8_t calculate(const uint8_t* data, size_t len,
                             const uint8_t init, const uint8_t polynomial,
                             const bool refIn, const bool refOut,
                             const uint8_t xorout, bool do_finalize = true) {
        uint8_t crc{init};
        while (len--) {
            uint8_t e = refIn ? reverseBitOrder(*data) : *data;
            ++data;
            crc ^= e;
            uint_fast8_t cnt{8};
            while (cnt--) {
                if (crc & 0x80) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            }
        }
        return do_finalize ? finalize(crc, refOut, xorout) : crc;
    }

   protected:
    static inline uint8_t finalize(const uint8_t value, const bool refOut,
                                   const uint8_t xorout) {
        return (refOut ? reverseBitOrder(value) : value) ^ xorout;
    }

   private:
    uint8_t _crc{}, _init{}, _polynomial{}, _xorout{};
    bool _refIn{}, _refOut{};
};

/*!
  @class CRC16
  @brief Base class of the CRC16 calculator
 */
class CRC16 {
   public:
    CRC16() = delete;
    /*!
      @param init Initial value
      @param polynormal Generated polynomial
      @param refIn Inverted input?
      @param refOut Inverted output?
      @param xorout Exclusive OR output
    */
    CRC16(const uint16_t init, const uint16_t polynomial, const bool refIn,
          const bool refOut, const uint16_t xorout)
        : _crc{init},
          _init{init},
          _polynomial{polynomial},
          _xorout{xorout},
          _refIn{refIn},
          _refOut{refOut} {
    }
    /*!
      @brief Calculate the CRC of the specified range
      @param data Pointer of the array
      @param len Length of the array
      @return CRC value
     */
    inline uint16_t range(const uint8_t* data, size_t len) {
        auto crc = calculate(data, len, _init, _polynomial, _refIn, _refOut,
                             _xorout, false);
        return finalize(crc, _refOut, _xorout);
    }
    /*!
      @brief Stores the CRC of the specified array using the current internal
      information
      @param data Pointer of the array
      @param len Length of the array
      @return CRC value
      @note Used when you want to calculate the value of the entire divided
      continuous data, such as streaming data
    */
    inline uint16_t update(const uint8_t* data, size_t len) {
        _crc = calculate(data, len, _crc, _polynomial, _refIn, _refOut, _xorout,
                         false);
        return finalize(_crc, _refOut, _xorout);
    }
    /*!
      @brief CRC value at the time of the call
      @return CRC value
     */
    inline uint16_t value() const {
        return finalize(_crc, _refOut, _xorout);
    }
    /*!
      @brief Calculate CRC16
      @param data Pointer of the array
      @param len Length of the array
      @param polynormal Generated polynomial
      @param refIn Inverted input?
      @param refOut Inverted output?
      @param xorout Exclusive OR output
      @param do_finalize Apply processing to output values?(true as defaut)
      @return CRC value
    */
    static uint16_t calculate(const uint8_t* data, size_t len,
                              const uint16_t init, const uint16_t polynomial,
                              const bool refIn, const bool refOut,
                              const uint16_t xorout, bool do_finalize = true) {
        uint16_t crc{init};
        while (len--) {
            uint8_t e{refIn ? reverseBitOrder(*data) : *data};
            ++data;
            crc ^= (e << 8);
            uint_fast8_t cnt{8};
            while (cnt--) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            }
        }
        return do_finalize ? finalize(crc, refOut, xorout) : crc;
    }

   protected:
    static inline uint16_t finalize(const uint16_t value, const bool refOut,
                                    const uint16_t xorout) {
        return (refOut ? reverseBitOrder(value) : value) ^ xorout;
    }

   private:
    uint16_t _crc{}, _init{}, _polynomial{}, _xorout{};
    bool _refIn{}, _refOut{};
};

/*!
  @class CRC8_CheckSum
  @brief Typical CRC8 calculator used for read and write data with the chip
*/
class CRC8_Checksum : public CRC8 {
   public:
    CRC8_Checksum() : CRC8(0xFF, 0x31, false, false, 0x00) {
    }
};

}  // namespace utility
}  // namespace m5

#endif
