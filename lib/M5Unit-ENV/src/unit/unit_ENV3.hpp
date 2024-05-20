/*!
  @file unit_ENV3.hpp
  @brief ENV III Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_ENV_UNIT_ENV3_HPP
#define M5_UNIT_ENV_UNIT_ENV3_HPP

#include <M5UnitComponent.hpp>
#include <array>
#include "unit_SHT3x.hpp"
#include "unit_QMP6988.hpp"

namespace m5 {
namespace unit {

/*!
  @class UnitENV3
  @brief ENV III is an environmental sensor that integrates SHT30 and QMP6988
  @details This unit itself has no I/O, but holds SHT30 and QMP6988
 */
class UnitENV3 : public Component {
   public:
    UnitSHT30 sht30;      //!< @brief SHT30 instance
    UnitQMP6988 qmp6988;  //!< @brief QMP6988 instance

    // Not access, UnitENV3 is just a container
    constexpr static uint8_t DEFAULT_ADDRESS{0x00};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitENV3(const uint8_t addr = DEFAULT_ADDRESS);
    virtual ~UnitENV3() {
    }

    virtual bool begin() override {
        return _valid;
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
    virtual Adapter* ensure_adapter(const uint8_t ch) override;

   private:
    bool _valid{};  // Did the constructor correctly add the child unit?
    std::array<std::unique_ptr<Adapter>, 2> _adapters{};
};

}  // namespace unit
}  // namespace m5
#endif
