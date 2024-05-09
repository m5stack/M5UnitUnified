/*!
  @file crc.hpp
  @brief Calculate CRC

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_CRC_HPP
#define M5_UTILITY_CRC_HPP

#include <cstdint>
#include <cstddef>

namespace m5 {
namespace utility {

/*!
  @class CRC8
  @brief Base
  @warning Not support Refin/out
 */
class CRC8 {
   public:
    ///@name Constructor
    //@{
    CRC8() = delete;
    CRC8(const uint8_t polynomial, const uint8_t xorout, const uint8_t init)
        : _polynomial(polynomial), _xorout{xorout}, _init(init) {
    }
    ///@}

    //! @brief Gets the CRC8
    uint8_t get(const uint8_t* data, size_t len) const {
        return calculate(data, len, _polynomial, _xorout, _init);
    }

    //! @brief Calculate CRC-8
    static uint8_t calculate(const uint8_t* data, size_t len,
                             const uint8_t polynomial, const uint8_t xorout,
                             const uint8_t init) {
        uint8_t crc = init;
        while (len--) {
            crc ^= *data++;
            uint_fast8_t cnt{8};
            while (cnt--) {
                if (crc & 0x80) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc ^ xorout;
    }

   private:
    uint8_t _polynomial{0x07};
    uint8_t _xorout{0x00};
    uint8_t _init{0x00};
};

/*!
  @class CRC8_Maxim
  @brief 1-Wire bus
*/
class CRC8_Maxim : public CRC8 {
   public:
    CRC8_Maxim() : CRC8(0x31, 0x00, 0xFF) {
    }
};

}  // namespace utility
}  // namespace m5

#endif
