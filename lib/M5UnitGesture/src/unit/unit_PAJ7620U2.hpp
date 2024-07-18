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
    Approach         = 1U << 9,   //!< Approach (Proximity mode)
    HasObject        = 1U << 10,  //!< Has object (cursor mode)
    WakeupTrigger    = 1U << 11,  //!< Wakeup (trigger mode)
    Confirm          = 1U << 12,  //!< Confirm (confirm mode)
    Abort            = 1U << 13,  //!< Abort (onfirm mode)
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
enum class Frequency : uint8_t {
    Normal,  //!< 120Hz
    Gaming,  //!< 240Hz
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
        paj7620u2::Mode mode{paj7620u2::Mode::Gesture};
        paj7620u2::Frequency frequency{paj7620u2::Frequency::Normal};
        bool hflip{false};
        bool vflip{true};
        uint8_t rotation{0};
    };

    explicit UnitPAJ7620U2(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitPAJ7620U2() {
    }

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

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

    ///@name Properties
    ///@{
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
    //! @brief Latest gesture
    inline paj7620u2::Gesture gesture() const {
        return _gesture;
    }
    /*!
      @brief Latest brightness
      @retval 0:Out of bounds
      @retval 1...255  [1:Far - Near:255]
      @note Return valid values if detection is Mode::Proximity
     */
    inline uint8_t brightness() const {
        return _mode == paj7620u2::Mode::Proximity ? _brightness : 0;
    }
    /*!
      @brief Latest approach status
      @retval true approach
      @reval false Not approach
      @note Return valid values if detection is Mode::Proximity
     */
    inline bool approach() const {
        return _mode == paj7620u2::Mode::Proximity ? _approach : false;
    }
    /*!
      @brief Latest has object
      @retval true has object
      @note Return valid values if detection is Mode::Cursor
    */
    bool hasObject() {
        return _mode == paj7620u2::Mode::Cursor
                   ? (_gesture == paj7620u2::Gesture::HasObject)
                   : false;
    }
    /*!
      @brief Latest cursorX
      @note Return valid values if detection is Mode::Cursor
     */
    inline uint16_t cursorX() const {
        return _mode == paj7620u2::Mode::Cursor ? _cursorX : 0xFFFF;
    }
    /*!
      @brief Latest cursorY
      @note Return valid values if detection is Mode::Cursor
     */
    inline uint16_t cursorY() const {
        return _mode == paj7620u2::Mode::Cursor ? _cursorY : 0xFFFF;
    }
    ///@}

    /*!
      @brief Get the rotation
      @sa setRotatation
    */
    inline uint8_t getRotation() const {
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

    /*! @brief Get the Frequency */
    inline paj7620u2::Frequency frequency() const {
        return _frequency;
    }
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
      @brief Gets the threshold for detect approch
      @param[out] high High threshold
      @param[out] low Lowthreshold
      @note Approach:brightness >= high
      @note Not approach: brightness <= low
      @warning Only valid in Proximity mode
     */
    bool getApprochThreshold(uint8_t& high, uint8_t& low);
    /*!
      @brief Sets the threshold for detect approch
      @param high High threshold
      @param low Lowthreshold
      @note Approach:brightness >= high
      @note Not approach: brightness <= low
      @warning Only valid in Proximity mode
     */
    bool setApprochThreshold(const uint8_t high, const uint8_t low);
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

    //! @brief Gets the horizontal flipping
    bool getHorizontalFlip(bool& flip);
    //! @brief Gets the vertical flipping
    bool getVerticalFlip(bool& flip);
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
    bool select_bank(const uint8_t bank, const bool force = false);

    bool read_banked_register(const uint16_t reg, uint8_t* buf,
                              const size_t len);
    bool read_banked_register8(const uint16_t reg, uint8_t& value);
    bool read_banked_register16(const uint16_t reg, uint16_t& value);
    bool write_banked_register(const uint16_t reg, const uint8_t* buf,
                               const size_t len);
    bool write_banked_register8(const uint16_t reg, const uint8_t value);
    bool write_banked_register16(const uint16_t reg, const uint16_t value);

    bool update_gesture();
    bool update_proximity();
    bool update_cursor();

    bool was_wakeup();
    bool get_chip_id(uint16_t& id);
    bool get_version(uint8_t& version);

   protected:
    uint8_t _current_bank{0xFF};
    paj7620u2::Mode _mode{};
    paj7620u2::Frequency _frequency{};
    uint8_t _rotation{};
    bool _updated{};
    unsigned long _latest{};

    // latest data
    // gesture
    paj7620u2::Gesture _gesture{paj7620u2::Gesture::None};
    // proximity
    uint8_t _brightness{};
    bool _approach{};
    // cursor
    uint16_t _cursorX{}, _cursorY{};

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
