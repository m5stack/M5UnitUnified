/*!
  @file unit_PaHub.hpp
  @brief PaHub Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_PAHUB_UNIT_PAHUB_HPP
#define M5_PAHUB_UNIT_PAHUB_HPP

#include <M5UnitComponent.hpp>
#include <array>

namespace m5 {
namespace unit {

/*!
  @class UnitPaHub
  @brief PaHub unit (PCA 9548AP)
 */
class UnitPaHub : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x70};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];
    constexpr static uint8_t MAX_CHANNEL = 6;

    explicit UnitPaHub(const uint8_t addr = DEFAULT_ADDRESS);
    virtual ~UnitPaHub() = default;

    UnitPaHub(UnitPaHub&& o);
    UnitPaHub& operator=(UnitPaHub&& o);

    uint8_t getCurrentChannel() const {
        return _current;
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
    virtual m5::hal::error::error_t select_channel(
        const uint8_t ch = 8) override;

   protected:
    std::array<std::unique_ptr<Adapter>, +MAX_CHANNEL>
        _adapters{};     // For children
    uint8_t _current{};  // current channel 0 ~ MAX_CHANNEL
};

}  // namespace unit
}  // namespace m5

#endif
