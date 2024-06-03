/*!
  @file unit_WS1850S.hpp
  @brief WS1850S Unit for M5UnitUnified

  Functionally compatible with MFRC522

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_RFID2_UNIT_WS1850S_HPP
#define M5_UNIT_RFID2_UNIT_WS1850S_HPP

#include "unit_MFRC522.hpp"

namespace m5 {
namespace unit {

/*!
  @class UnitWS1850S
  @brief Radio frequency identification unit
  @details Functionally compatible with MFRC522
 */
class UnitWS1850S : public UnitMFRC522 {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x28};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitWS1850S(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitMFRC522(addr) {
    }
    virtual ~UnitWS1850S() {
    }

    virtual bool begin() override;
    virtual void update() override;

    /*!
      @brief self test
      @return always false
      @warning It is recommended not to use this feature in the WS1850S as,
      unlike in the MFRC522, self-testing is not successful.
     */
    inline virtual bool selfTest() override {
        return UnitMFRC522::selfTest();
    }

   protected:
    inline virtual const char* unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }
};

}  // namespace unit
}  // namespace m5
#endif
