/*!
  @file crc.hpp
  @brief Calculate CRC

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
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
  @brief Base
 */
class CRC8 {
   public:
    ///@name Constructor
    //@{
    CRC8() = delete;
    CRC8(const uint8_t init, const uint8_t polynomial, const bool refIn,
         const bool refOut, const uint8_t xorout)
        : _init{init},
          _polynomial{polynomial},
          _refIn{refIn},
          _refOut{refOut},
          _xorout{xorout} {
    }
    ///@}

    //! @brief Gets the CRC8
    inline uint8_t get(const uint8_t* data, size_t len) const {
        return calculate(data, len, _init, _polynomial, _refIn, _refOut,
                         _xorout);
    }

    //! @brief Calculate CRC-8
    static uint8_t calculate(const uint8_t* data, size_t len,
                             const uint8_t init, const uint8_t polynomial,
                             const bool refIn, const bool refOut,
                             const uint8_t xorout) {
        uint8_t crc = init;
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
        if (refOut) {
            crc = reverseBitOrder(crc);
        }
        return crc ^ xorout;
    }

   private:
    uint8_t _init{0x00};
    uint8_t _polynomial{0x07};
    bool _refIn{}, _refOut{};
    uint8_t _xorout{0x00};
};

/*!
  @class CRC8_Maxim
  @brief 1-Wire bus
*/
class CRC8_Maxim : public CRC8 {
   public:
    CRC8_Maxim() : CRC8(0xFF, 0x31, false, false, 0x00) {
    }
};

/*!
  @class CRC16
  @brief Base
 */
class CRC16 {
   public:
    ///@name Constructor
    //@{
    CRC16() = delete;
    CRC16(const uint16_t init, const uint16_t polynomial, const bool refIn,
          const bool refOut, const uint16_t xorout)
        : _init{init},
          _polynomial{polynomial},
          _refIn{refIn},
          _refOut{refOut},
          _xorout{xorout} {
    }
    ///@}

    //! @brief Gets the CRC16
    inline uint16_t get(const uint8_t* data, size_t len) const {
        return calculate(data, len, _init, _polynomial, _refIn, _refOut,
                         _xorout);
    }

    //! @brief Calculate CRC-16
    static uint16_t calculate(const uint8_t* data, size_t len,
                              const uint16_t init, const uint16_t polynomial,
                              const bool refIn, const bool refOut,
                              const uint16_t xorout) {
        uint16_t crc = init;
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
        if (refOut) {
            crc = reverseBitOrder(crc);
        }
        return crc ^ xorout;
    }

   private:
    uint16_t _init{0x0000};
    uint16_t _polynomial{0x1021};
    bool _refIn{}, _refOut{};
    uint16_t _xorout{0x0000};
};

}  // namespace utility
}  // namespace m5

#endif
