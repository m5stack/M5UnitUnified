/*!
  @file unit_PAJ7620U2.hpp
  @brief PAJ7620U2 Unit for M5UnitUnified

  @mainpage M5UnitGesture

  PAJ7620U2 touchless Gesture Moving Recognition Sensor Recognize up to 9
  Gestures It's a touchless 3D gesture recognition sensor via the I2C interface
  that integrates with PAJ7620U2. You could feel the charm of 9 kinds of gesture
  recognition by default program. In addition to adding more complex gestures,
  the recognition rate is up to 240Hz with higher anti-interference capability.
  This sensor is reliable for its recognition process is intuitive and has low
  power consumption (operating current only 2.2mA) that allows you to control a
  robot in accordance with your hand's movement. This is particularly useful in
  interactive applications.

  Support 9 gestures:
  Up, Down, Left, Right, Front, Back, Clockwise, Counterclockwise, Quick wave

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_GESTURE_UNIT_PAJ7620U2_HPP
#define M5_UNIT_GESTURE_UNIT_PAJ7620U2_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

namespace paj7620u2 {
/*!
  @enum Gesture
  @brief Gesture type
  @details Same as gesture detection interrupt flag mask(except Wave)
  bank:0 reg:0x41,0x42
 */
enum class Gesture : uint16_t {
    None,                        //!< No gesture
    Up               = 1U << 0,  //!< To up
    Down             = 1U << 1,  //!< To down
    Left             = 1U << 2,  //!< To left
    Right            = 1U << 3,  //!< To right
    Forward          = 1U << 4,  //!< Closer to the sensor
    Backward         = 1U << 5,  //!< Away from the sensor
    Clockwise        = 1U << 6,  //!< Clockwise
    CounterClockwise = 1U << 7,  //!< Counter clock wise
    Wave             = 1U << 8,  //!< Wave
};

/*!
  @enum Mode
  @brief behaviour mode
 */
enum class Mode : uint8_t {
    Gesture,    //!< Detect gesture
    Proximity,  //!< Detect proximity (PS mode)
};

}  // namespace paj7620u2

/*!
  @class UnitPAJ7620U2
  @brief PAJ7620U2 unit
 */
class UnitPAJ7620U2 : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitPAJ7620U2, 0x73);

   public:
    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
        //! @brief behaviour mode
        paj7620u2::Mode mode{paj7620u2::Mode::Gesture};
    };

    explicit UnitPAJ7620U2(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitPAJ7620U2() {
    }

    virtual bool begin() override;
    virtual void update() override;

    ///@name Settings
    ///@{
    /*! @brief Gets the configration */
    config_t config() {
        return _cfg;
    }
    //! @brief Set the configration
    void config(const config_t& cfg) {
        _cfg = cfg;
    }
    ///@}

    //! @brief Periodic measurement data updated?
    inline bool updated() const {
        return _updated;
    }
    /*!
      @brief Time elapsed since start-up when the measurement data was updated
      in update()
      @return Updated time (Unit: ms)
    */
    inline unsigned long updatedMillis() const {
        return _latest;
    }
    inline paj7620u2::Gesture gesture() const {
        return _gesture;
    }

    ///@name Behaviour mode
    ///@{
    /*! @brief Gets the mode */
    inline paj7620u2::Mode mode() const {
        return _mode;
    }
    /*!
      @brief Sets the mode
      @param mode behaviour mode
      @return True if successful
     */
    bool setMode(const paj7620u2::Mode m);
    ///@}

    ///@name General purpose
    ///@{
    /*!
      @brief enable/disable sensor
      @param flag true:enable false:disable
      @return True if successful
     */
    bool enableSensor(const bool flag);
    //! @brief enable sensor
    inline bool enableSensor() {
        return enableSensor(true);
    }
    //! @brief disable sensor
    inline bool disableSensor() {
        return enableSensor(false);
    }
    //! @brief suspend
    bool suspend();
    //! @brief resume from suspended
    bool resume();
    ///@}

   protected:
    bool select_bank(const uint8_t bank, const bool force = false);

    bool read_banked_register(const uint16_t reg, uint8_t* buf,
                              const size_t len);
    bool read_banked_register8(const uint16_t reg, uint8_t& value);
    bool read_banked_register16(const uint16_t reg, uint16_t& value);
    bool write_banked_register(const uint16_t reg, const uint8_t* buf,
                               const size_t len);
    bool write_banked_register8(const uint16_t reg, const uint8_t value);
    bool write_banked_register16(const uint16_t reg, const uint16_t value);

    bool get_chip_id(uint16_t& id);
    bool get_version(uint8_t& version);

    bool read_detection(uint16_t& detection);

   protected:
    uint8_t _current_bank{0xFF};
    paj7620u2::Mode _mode{};
    bool _updated{};
    unsigned long _latest{};

    paj7620u2::Gesture _gesture{paj7620u2::Gesture::None};

    config_t _cfg{};
};

namespace paj7620u2 {
namespace command {
///@eond 0
// Bnak 0/1
constexpr uint8_t BANK_SEL{0xEF};
// high byte:bank low byte: register
// Bank 0
constexpr uint16_t PART_ID_LOW{0x00};
constexpr uint16_t PART_ID_HIGH{0x01};
constexpr uint16_t VERSION_ID{0x02};
constexpr uint16_t SUSPEND_COMMAND{0x03};
constexpr uint16_t GESTURE_DETECTION_INTERRUPT_FLAG_LOW{0x43};
constexpr uint16_t GESTURE_DETECTION_INTERRUPT_FLAG_HIGH{0x44};

// Bank1
constexpr uint16_t OPERATION_ENABLE{0x0172};
/// @endcond
}  // namespace command
}  // namespace paj7620u2

}  // namespace unit
}  // namespace m5

#endif
