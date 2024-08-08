/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
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
*/
#ifndef M5_UNIT_GESTURE_UNIT_PAJ7620U2_HPP
#define M5_UNIT_GESTURE_UNIT_PAJ7620U2_HPP

#include <M5UnitComponent.hpp>
#include <m5_utility/container/circular_buffer.hpp>

namespace m5 {
namespace unit {

/*!
  @namespace paj7620u2
  @brief For PAJ7620U2
 */
namespace paj7620u2 {
/*!
  @enum Gesture
  @brief Gesture type
  @details Same as gesture detection interrupt flag
  bank:0 reg:0x43,0x44
 */
enum class Gesture : uint16_t {
    None,                         //!< No gesture
    Up               = 1U << 0,   //!< To up
    Down             = 1U << 1,   //!< To down
    Left             = 1U << 2,   //!< To left
    Right            = 1U << 3,   //!< To right
    Forward          = 1U << 4,   //!< Closer to the sensor
    Backward         = 1U << 5,   //!< Away from the sensor
    Clockwise        = 1U << 6,   //!< Clockwise
    CounterClockwise = 1U << 7,   //!< Counter clock wise
    Wave             = 1U << 8,   //!< Wave
    Approach         = 1U << 9,   //!< Approach (proximity mode)
    HasObject        = 1U << 10,  //!< Has object (cursor mode)
    WakeupTrigger    = 1U << 11,  //!< Wakeup (trigger mode)
    Confirm          = 1U << 12,  //!< Confirm (confirm mode)
    Abort            = 1U << 13,  //!< Abort (confirm mode)
    Reserve          = 1U << 14,
    NoObject         = 1U << 15,  //!< No object (cursor mode)
};

/*!
  @enum Mode
  @brief What it detects
 */
enum class Mode : uint8_t {
    Gesture,    //!< Detect gesture
    Proximity,  //!< Detect proximity
    Cursor,     //!< Detect XY cordinate
};

/*!
  @brief Frequency
  Operating frequency
*/
enum class Frequency : int8_t {
    Unknown = -1,  //!< Unknown freq
    Normal,        //!< 120Hz
    Gaming,        //!< 240Hz
};

/*!
  @struct Data
  @brief Measurement data group
 */
struct Data {
    // Common
    // [0,1]:gesture
    // Proximity
    // [2]:proximity [3]:approach
    // Cirosr
    // [2,3]:X [4,5]:Y
    std::array<uint8_t, 2 + 4> raw{};
    Gesture data_gesture{};
    Mode data_mode{};
    union {
        struct {
            uint8_t proximity_brightness;
            bool proximity_approach;
        };
        struct {
            uint16_t cursor_x{}, cursor_y{};
        };
    };

    //! @brief Gets the data mode
    inline Mode mode() const {
        return data_mode;
    }
    //! @brief Gets the gesture (Common)
    Gesture gesture() const {
        return data_gesture;
    }
    ///@name Proximity mode
    ///@{
    /*! @brief Gets the brightness */
    inline uint8_t brightness() const {
        return (data_mode == Mode::Proximity) ? proximity_brightness : 0;
    }
    /*! @brief Detect the approach? */
    inline bool approach() const {
        return (data_mode == Mode::Proximity) ? proximity_approach : false;
    }
    ///@}
    ///@name Cursor mode
    ///@{
    /*! @brief Has object? */
    inline bool hasObject() const {
        return (data_mode == Mode::Cursor) ? gesture() == Gesture::HasObject
                                           : false;
    }
    /*! @brief Gets the cursor X the any object */
    inline uint16_t cursorX() const {
        return (data_mode == Mode::Cursor) ? cursor_x : 0xFFFF;
    }
    /*! @brief Gets the cursor Y the any object */
    inline uint16_t cursorY() const {
        return (data_mode == Mode::Cursor) ? cursor_y : 0xFFFF;
    }
    ///@}
};

}  // namespace paj7620u2


/*!
  @class UnitPAJ7620U2
  @brief PAJ7620U2 unit
 */
class UnitPAJ7620U2
    : public Component,
      public PeriodicMeasurementAdapter<UnitPAJ7620U2, paj7620u2::Data> {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitPAJ7620U2, 0x73);

   public:
    /*!
      @struct config_t
      @brief Settings for begin
     */
    struct config_t : public Component::config_t {
        //! @brief Start periodic measurement on begin?
        bool start_periodic{true};
        //! @brief Set mode on begin
        paj7620u2::Mode mode{paj7620u2::Mode::Gesture};
        //! bbrief Set freq o begin
        paj7620u2::Frequency frequency{paj7620u2::Frequency::Normal};
        //! @brief Flip horizontal on begin
        bool hflip{false};
        //! @brief Flip verticl on begin
        bool vflip{true};
        //! @brief Set rotation on begin
        uint8_t rotation{0};
        //! Store only when value is a change
        bool store_on_change{true};
    };

    explicit UnitPAJ7620U2(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr),
          _data{new m5::container::CircularBuffer<paj7620u2::Data>(1)} {
    }
    virtual ~UnitPAJ7620U2() {
    }

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

    ///@name Settings for begin
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

    /*! @fn  bool PeriodicMeasurementAdapter::startPeriodicMeasurement()
     * @brief TEST
     */

    ///@name Measurement data by periodic
    ///@{
    /*! @brief Oldest gesture */
    inline paj7620u2::Gesture gesture() const {
        return !empty() ? oldest().gesture() : paj7620u2::Gesture::None;
    }
    //! @brief Oldest brightness if Proximity mode
    inline uint8_t brightness() const {
        return !empty() ? oldest().brightness() : 0;
    }
    //! @brief Oldest approach status if Proximity mode
    inline bool approach() const {
        return !empty() ? oldest().approach() : false;
    }
    //! @brief Oldest has object status if Cursor mode
    bool hasObject() {
        return !empty() ? oldest().hasObject() : false;
    }
    //! @brief Oldest cursor X if Cursor mode
    uint16_t cursorX() const {
        return !empty() ? oldest().cursorX() : 0xFFFF;
    }
    //! @brief Oldest cursor Y if Cursor mode
    uint16_t cursorY() const {
        return !empty() ? oldest().cursorY() : 0xFFFF;
    }
    ///@}

    /*!
      @brief Get the rotation
      @return Rotation [0...3]
      @sa setRotatation
    */
    inline uint8_t rotation() const {
        return _rotation;
    }
    /*!
      @brief Set the roptation
      @param rot Rotation [0...3]
      ```
      +-------++
      |||     ||   rot:0  up
      |||   O  |      left  right
      +--------+         down
      rot 1,2,3... Treat as 90 deg counter-clockwise rotation
      ```
     */
    void setRotate(const uint8_t rot);

    //! @brief Gets the inner frequency
    paj7620u2::Frequency frequency() const {
        return _frequency;
    }
    /*!
      @brief Read the raw frequency
      @param[out] raw raw frequency
      @return True if successful
     */
    bool readFrequency(uint8_t& raw);
    /*!
      @brief Read the frequency
      @param[out] f Frequency
      @return True if successful
     */
    bool readFrequency(paj7620u2::Frequency& f);
    /*!
      @brief Set the frequency
      @param f Frequency
      @return True if successful
    */
    bool setFrequency(const paj7620u2::Frequency f);

    ///@name For detect gesture
    ///@{
    /*!
      @brief Read gesture
      @param[out] gesture
      @return True if successful
    */
    bool readGesture(paj7620u2::Gesture& gesture);
    /*!
      @brief Object center position
      @param[out] x X cordinate
      @param[out] y Y cordinate
      @return True if successful
     */
    bool readObjectCenter(uint16_t& x, uint16_t& y);
    /*!
      @brief Read gesture object size
      @param[out] sz size [0~900]
      @return True if successful
     */
    bool readObjectSize(uint16_t& sz);
    /*!
      @brief Is the object present within the detection range?
      @param[out] exists true if exists
      @return True if successful
    */
    bool existsObject(bool& exists) {
        uint8_t e{};
        if (readNoObjectCount(e)) {
            exists = (e == 0);
            return true;
        }
        return false;
    }
    /*!
      @brief State counter with no objects detected
      @param[out] count(tiocks) [0...255]
      @return True if successful
    */
    bool readNoObjectCount(uint8_t& cnt);
    /*!
      @brief State counter which a non-moving object is detected
      @param[out] count(tiocks) [0...12]
      @return True if successful
    */
    bool readNoMotionCount(uint8_t& cnt);
    ///@}

    ///@name For detect proximity
    ///@{
    /*!
      @brief Read proximity
      @param[out] brightness 0:Out of bounds  [1:far ... 255:near]
      @param[out] approarch Approach object
      @return True if successful
     */
    bool readProximity(uint8_t& brightness, uint8_t& approach);

    /*!
      @brief Read the threshold for detect approach
      @param[out] high High threshold
      @param[out] low Lowthreshold
      @note Approach:brightness >= high
      @note Not approach: brightness <= low
      @warning Only valid in Proximity mode
     */
    bool readApproachThreshold(uint8_t& high, uint8_t& low);
    /*!
      @brief Sets the threshold for detect approach
      @param high High threshold
      @param low Lowthreshold
      @note Approach:brightness >= high
      @note Not approach: brightness <= low
      @warning Only valid in Proximity mode
     */
    bool setApproachThreshold(const uint8_t high, const uint8_t low);
    ///@}

    ///@name For detect cursor
    ///@{
    /*!
      @brief Cursor position
      @param[out] x X cordinate
      @param[out] y Y cordinate
      @return True if successful
      @warning Only valid in Cursor mode
    */
    bool readCursor(uint16_t& x, uint16_t& y);
    ///@}

    ///@name Detection mode
    ///@{
    /*! @brief Gets the detection mode */
    inline paj7620u2::Mode mode() const {
        return _mode;
    }
    /*!
      @brief Sets the detection mode
      @param mode detection mode
      @return True if successful
     */
    bool setMode(const paj7620u2::Mode m);
    ///@}

    //! @brief Read the horizontal flipping
    bool readHorizontalFlip(bool& flip);
    //! @brief Read the vertical flipping
    bool readVerticalFlip(bool& flip);
    //! @brief Sets the horizontal flipping
    bool setHorizontalFlip(const bool flip);
    //! @brief Sets the vertical flipping
    bool setVerticalFlip(const bool flip);

    ///@name General purpose
    ///@{
    /*!
      @brief Enable/disable sensor
      @param flag true:enable false:disable
      @return True if successful
     */
    bool enable(const bool flag);
    //! @brief enable sensor
    inline bool enable() {
        return enable(true);
    }
    //! @brief disable sensor
    inline bool disable() {
        return enable(false);
    }
    //! @brief suspend
    bool suspend();
    //! @brief resume from suspended
    bool resume();
    ///@}

   protected:
    ///@note Call via startPeriodicMeasurement/stopPeriodicMeasurement
    ///@name Periodic measurement
    ///@{
    bool start_periodic_measurement(const uint32_t intervalMs = 0);

    bool start_periodic_measurement(const paj7620u2::Mode mode,
                                    const paj7620u2::Frequency freq,
                                    const uint32_t intervalMs);

    bool stop_periodic_measurement();
    ///@}
    M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(UnitPAJ7620U2,
                                                               paj7620u2::Data);

    bool select_bank(const uint8_t bank, const bool force = false);

    bool read_banked_register(const uint16_t reg, uint8_t* buf,
                              const size_t len);
    bool read_banked_register8(const uint16_t reg, uint8_t& value);
    bool read_banked_register16(const uint16_t reg, uint16_t& value);
    bool write_banked_register(const uint16_t reg, const uint8_t* buf,
                               const size_t len);
    bool write_banked_register8(const uint16_t reg, const uint8_t value);
    bool write_banked_register16(const uint16_t reg, const uint16_t value);

    bool update_gesture(paj7620u2::Data& d);
    bool update_proximity(paj7620u2::Data& d);
    bool update_cursor(paj7620u2::Data& d);

    bool read_gesture(paj7620u2::Data& d);
    bool read_proximity(paj7620u2::Data& d);
    bool read_cursor(paj7620u2::Data& d);

    bool was_wakeup();
    bool get_chip_id(uint16_t& id);
    bool get_version(uint8_t& version);

   protected:
    uint8_t _current_bank{0xFF};
    paj7620u2::Mode _mode{};
    paj7620u2::Frequency _frequency{};
    uint8_t _rotation{};

    std::unique_ptr<m5::container::CircularBuffer<paj7620u2::Data>> _data{};
    config_t _cfg{};
};

///@cond 0
namespace paj7620u2 {
namespace command {

// Bnak 0/1
constexpr uint8_t BANK_SEL{0xEF};
// high byte:bank low byte: register
// Bank 0
constexpr uint16_t PART_ID_LOW{0x0000};
constexpr uint16_t PART_ID_HIGH{0x0001};
constexpr uint16_t VERSION_ID{0x0002};
constexpr uint16_t SW_SUSPEND_ENL{0x0003};

constexpr uint16_t CURSOR_CLAMP_CENTER_X_LOW{0X003B};
constexpr uint16_t CURSOR_CLAMP_CENTER_X_HIGH{0X003C};
constexpr uint16_t CURSOR_CLAMP_CENTER_Y_LOW{0X003D};
constexpr uint16_t CURSOR_CLAMP_CENTER_Y_HIGH{0X003E};

constexpr uint16_t INT_FLAG_1{0x0043};
constexpr uint16_t INT_FLAG_2{0x0044};

const uint16_t R_POX_UB{0X0069};
const uint16_t R_POX_LB{0X006A};
constexpr uint16_t S_STATE{0x006B};
constexpr uint16_t S_AVGY{0x006C};

constexpr uint16_t OBJECT_CENTER_X_LOW{0X00AC};
constexpr uint16_t OBJECT_CENTER_X_HIGH{0X00AD};
constexpr uint16_t OBJECT_CENTER_Y_LOW{0X00AE};
constexpr uint16_t OBJECT_CENTER_Y_HIGH{0X00AF};

constexpr uint16_t OBJECT_AVG_Y{0X00B0};
constexpr uint16_t OBJECT_SIZE_LOW{0x00B1};
constexpr uint16_t OBJECT_SIZE_HIGH{0x00B2};
constexpr uint16_t WAVE_ABORT_COUNT{0x00B7};
constexpr uint16_t NO_OBJECT_COUNT{0x00B8};
constexpr uint16_t NO_MOTION_COUNT{0x00B9};

constexpr uint16_t VEL_X_LOW{0x00C3};
constexpr uint16_t VEL_X_HIGH{0x00C4};
constexpr uint16_t VEL_Y_LOW{0x00C3};
constexpr uint16_t VEL_Y_HIGH{0x00C4};

// Bank1
constexpr uint16_t LS_COMP_DAVG_V{0x0104};
constexpr uint16_t R_REF_CLK_CNT_LOW{0x0165};
constexpr uint16_t R_TG_ENH{0x0172};
}  // namespace command
}  // namespace paj7620u2
/// @endcond

}  // namespace unit
}  // namespace m5

#endif
