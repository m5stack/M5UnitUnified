/*!
  @file unit_QMP6988.hpp
  @brief QMP6988 Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UNIT_ENV_UNIT_QMP6988_HPP
#define M5_UNIT_ENV_UNIT_QMP6988_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

namespace qmp6988 {
/*!
  @enum Average
  @brief Average times setting for oversampling
 */
enum class Average : uint8_t {
    Skip,   //!< @brief No measurement
    Avg1,   //!< @brief 1 time
    Avg2,   //!< @brief 2 times
    Avg4,   //!< @brief 4 times
    Avg8,   //!< @brief 8 times
    Avg16,  //!< @brief 16 times
    Avg32,  //!< @brief 32 times
    Avg64,  //!< @brief 64 times
};

/*!
  @enum PowerMode
  @brief Operation mode setting
 */
enum class PowerMode : uint8_t {
    Sleep =
        0,  //!< @brief Minimal power consumption, but no measurements are taken
    Force =
        1,  //!< @brief Energise the circuit for measurement only when measuring
    Normal = 3,  //!< @brief Normally energized
};

/*!
  @struct Status
  @brief Accessor for Status
 */
struct Status {
    //! @brief Device operation status. T
    inline bool measure() const {
        return value & (1U << 3);
    }
    // @brief the status of OTP data access
    inline bool OTP() const {
        return value & (1U << 0);
    }

    uint8_t value{};
};

};  // namespace qmp6988

/*!
  @class UnitQMP6988
  @brief Barometric pressure sensor to measure atmospheric pressure and altitude
  estimation
*/
class UnitQMP6988 : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x70};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitQMP6988(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitQMP6988() {
    }

    virtual bool begin() override;
    virtual void update() override;

    // API

    ///@name Measurement condition
    ///@{
    bool getMeasurementCondition(qmp6988::Average& ta, qmp6988::Average& pa,
                                 qmp6988::PowerMode& mode);
    bool setMeasurementCondition(const qmp6988::Average ta,
                                 const qmp6988::Average pa,
                                 const qmp6988::PowerMode mode);
    bool setPowerMode(const qmp6988::PowerMode mode);
    bool setPresureOversampling(const qmp6988::Average a);
    bool setTemperatureOversampling(const qmp6988::Average a);
    ///@}

    bool reset();
    bool getStatus(qmp6988::Status& s);

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

namespace qmp6988 {
namespace command {

constexpr uint8_t CONTROL_MEASUREMENT{0xF4};
constexpr uint8_t GET_STATUS{0xF3};
constexpr uint8_t RESET{0xE0};

}  // namespace command
}  // namespace qmp6988

}  // namespace unit
}  // namespace m5
#endif
