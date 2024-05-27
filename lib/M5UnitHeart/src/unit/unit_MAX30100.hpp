/*!
  @file unit_MAX30100.hpp
  @brief MAX30100 Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_ENV_UNIT_MAX30100_HPP
#define M5_UNIT_ENV_UNIT_MAX30100_HPP

#include <M5UnitComponent.hpp>
#include "m5_utility/stl/extension.hpp"

namespace m5 {
namespace unit {

namespace max30100 {
/*!
  @enum Mode
  @brief For Mode control
 */
enum class Mode : uint8_t {
    HROnly = 0x02,  //!< @brief HR only enabled
    SPO2,           //!< @brief SPO2 and HR enabled
};

/*!
  @struct ModeConfiguration
  @brief Accessor for ModeConfiguration
 */
struct ModeConfiguration {
    ///@name Getter
    ///@{
    bool shdn() const {
        return value & (1U << 7);
    }  //!< @brief Shutdown Control (SHDN)
    bool reset() const {
        return value & (1U << 6);
    }  //!< @brief Reset control
    bool temperature() const {
        return value & (1U << 3);
    }  //!< @brief Temperature enable
    Mode mode() const {
        return static_cast<Mode>(value & 0x07);
    }  //!< @brief Mode control
    ///@}

    ///@name Setter
    ///@{
    void shdn(const bool b) {
        value = (value & ~(1U << 7)) | ((b ? 1 : 0) << 7);
    }  //!< @brief Shutdown Control (SHDN)
    void reset(const bool b) {
        value = (value & ~(1U << 6)) | ((b ? 1 : 0) << 6);
    }  //!< @brief Reset control
    void temperature(const bool b) {
        value = (value & ~(1U << 3)) | ((b ? 1 : 0) << 3);
    }  //!< @brief Temperature enable
    void mode(const Mode m) {
        value = (value & ~0x07) | (m5::stl::to_underlying(m) & 0x07);
    }  //!< @brief Mode control
    ///@}

    uint8_t value{};
};

/*!
  @enum SamplingRate
  @brief Sample rate for pulse
  @details Unit is the number of sample per second
 */
enum class SamplingRate : uint8_t {
    Sampling50,    //!< @brief 50 sps
    Sampling100,   //!< @brief 100 sps
    Sampling167,   //!< @brief 167 sps
    Sampling200,   //!< @brief 200 sps
    Sampling400,   //!< @brief 400 sps
    Sampling600,   //!< @brief 600 sps
    Sampling800,   //!< @brief 800 sps
    Sampling1000,  //!< @brief 1000 sps
};

/*!
  @enum LedPulseWidth
  @brief  LED pulse width (the IR and RED have the same pulse width)
*/
enum class LedPulseWidth {
    PW200,   //!< @brief 200 us (ADC 13 bits)
    PW400,   //!< @brief 400 us (ADC 14 bits)
    PW800,   //!< @brief 800 us (ADC 15 bits)
    PW1600,  //!< @brief 1600 us (ADC 16 bits)
};

/*!
  @struct SpO2Configuration
  @brief Accessor for SpO2Configuration
  @warning Note that there are different combinations that can be set depending
  on the mode

  - Mode:SPO2
  |Sample\PulseWidth|200 |400 |800 |1600|
  |---|---|---|---|---|
  |  50|o|o|o|o|
  | 100|o|o|o|o|
  | 167|o|o|o|x|
  | 200|o|o|o|x|
  | 400|o|o|x|x|
  | 600|o|x|x|x|
  | 800|o|x|x|x|
  |1000|o|x|x|x|
  - Mode:HROnly
  |Sample\PulseWidth|200 |400 |800 |1600|
  |---|---|---|---|---|
  |  50|o|o|o|o|
  | 100|o|o|o|o|
  | 167|o|o|o|x|
  | 200|o|o|o|x|
  | 400|o|o|x|x|
  | 600|o|o|x|x|
  | 800|o|o|x|x|
  |1000|o|o|x|x|
*/
struct SpO2Configuration {
    ///@name Getter
    ///@{
    bool highResolution() const {
        return value & (1U << 6);
    }
    SamplingRate samplingRate() const {
        return static_cast<SamplingRate>((value >> 2) & 0x07);
    }
    LedPulseWidth ledPulseWidth() const {
        return static_cast<LedPulseWidth>(value & 0x03);
    }
    ///@}

    ///@name Setter
    ///@{
    void highResolution(const bool b) {
        value = (value & ~(1U << 6)) | ((b ? 1 : 0) << 6);
    }
    void samplingRate(const SamplingRate rate) {
        value = (value & ~(0x07 << 2)) |
                ((m5::stl::to_underlying(rate) & 0x07) << 2);
    }
    void ledPulseWidth(const LedPulseWidth width) {
        value = (value & ~0x03) | (m5::stl::to_underlying(width) & 0x03);
    }
    ///@}

    uint8_t value{};
};  // namespace max30100

/*!
  @enum CurrentControl
  @brief Current level (Unit mA)
 */
enum class CurrentControl {
    mA0_0,   //!< @brief 0,0 mA
    mA4_4,   //!< @brief 4,4 mA
    mA7_6,   //!< @brief 7.6 mA
    mA11_0,  //!< @brief 11.0 mA
    mA14_2,  //!< @brief 14.2 mA
    mA17_4,  //!< @brief 17.4 mA
    mA20_8,  //!< @brief 20,8 mA
    mA24_0,  //!< @brief 24.0 mA
    mA27_1,  //!< @brief 27.1 mA
    mA30_6,  //!< @brief 30.6 mA
    mA33_8,  //!< @brief 33.8 mA
    mA37_0,  //!< @brief 37.0 mA
    mA40_2,  //!< @brief 40.2 mA
    mA43_6,  //!< @brief 43.6 mA
    mA46_8,  //!< @brief 46.8 mA
    mA50_0,  //!< @brief 50.0 mA
};

/*!
  @struct LedConfiguration
  @brief Accessor for LedConfiguration
 */
struct LedConfiguration {
    ///@name Getter
    ///@{
    CurrentControl redLed() const {
        return static_cast<CurrentControl>((value >> 4) & 0x0F);
    }
    CurrentControl irLed() const {
        return static_cast<CurrentControl>(value & 0x0F);
    }
    ///@}
    ///@name Setter
    ///@{
    void redLed(const CurrentControl cc) {
        value =
            (value & ~(0x0F << 4)) | ((m5::stl::to_underlying(cc) & 0x0F) << 4);
    }
    void irLed(const CurrentControl cc) {
        value = (value & ~0x0F) | (m5::stl::to_underlying(cc) & 0x0F);
    }
    ///@}
    uint8_t value{};
};

}  // namespace max30100

/*!
  @class UnitMAX30100
  @brief Pulse oximetry and heart-rate sensor
*/
class UnitMAX30100 : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x57};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

#if 0
    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
        //! @brief Start periodic measurement on begin?
        bool start_periodic{true};
        //! @brief using low power mode if start peridodic on begin?
        bool low_power{false};
        //! @brief  Enable calibration on begin?
        bool auto_calibration{true};
    };
#endif

    explicit UnitMAX30100(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitMAX30100() {
    }

    virtual bool begin() override;
    virtual void update() override;

#if 0    
    ///@name Settings
    ///@{
    /*! @brief Gets the configration */
    config_t config() {
        return _cfg;
    }
    //! @brief Set the configration
    void config(const config_t &cfg) {
        _cfg = cfg;
    }
    ///@}
#endif

    // API
    ///@name Status
    ///@{
    //    bool readInterruptStatus();
    ///@}

    ///@name Mode Configuration
    ///@warning Note that there are different combinations that can be set
    /// depending on the mode See also SpO2Configuration
    ///@{
    bool getModeConfiguration(max30100::ModeConfiguration& mc);
    bool setModeConfiguration(const max30100::ModeConfiguration mc);
    bool setMode(const max30100::Mode mode);
    bool enablePowerSave() {
        return enable_power_save(true);
    }
    bool disablePowerSave() {
        return enable_power_save(false);
    }
    ///@}

    ///@name SpO2 Configuration
    ///@warning Note that there are different combinations that can be set
    /// depending on the mode See also SpO2Configuration
    ///@{
    bool getSpO2Configuration(max30100::SpO2Configuration& sc);
    bool setSpO2Configuration(const max30100::SpO2Configuration sc);
    bool setSamplingRate(const max30100::SamplingRate rate);
    bool setLedPulseWidth(const max30100::LedPulseWidth width);
    inline bool enableHighResolution() {
        return enable_high_resolution(true);
    }
    inline bool disableHighResolution() {
        return enable_high_resolution(false);
    }
    ///@}

    ///@name LED Configuration
    ///@{
    bool getLedConfiguration(max30100::LedConfiguration& lc);
    bool setLedConfiguration(const max30100::LedConfiguration lc);
    bool setLedCurrent(const max30100::CurrentControl ir,
                       const max30100::CurrentControl red);
    ///@}

    bool reset();

    bool readFIFOData();

    ///@name Temperature
    ///@{
    bool measurementTemperature();
    bool isMeasurementTemperature();
    bool readMeasurementTemperature(float& temp);
    ///@}

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

    bool get_mode_configration(uint8_t& c);
    bool set_mode_configration(const uint8_t c);
    bool enable_power_save(const bool enabled);
    bool get_spo2_configration(uint8_t& c);
    bool set_spo2_configration(const uint8_t c);
    bool enable_high_resolution(const bool enabled);
    bool get_led_configration(uint8_t& c);
    bool set_led_configration(const uint8_t c);
};

///@cond
namespace max30100 {

constexpr uint8_t MAX_FIFO_DEPTH{16};

namespace command {
// STATUS
constexpr uint8_t INTERRUPT_STATUS{0x00};
constexpr uint8_t INTERRUPT_ENABLE{0x01};
// FIFO
constexpr uint8_t FIFO_WRITE_POINTER{0x02};
constexpr uint8_t OVER_FLOW_COUNTER{0x03};
constexpr uint8_t FIFO_READ_POINTER{0x04};
constexpr uint8_t FIFO_DATA_REGISTER{0x05};
// CONFIGURATION
constexpr uint8_t MODE_CONFIGURATION{0x06};
constexpr uint8_t SPO2_CONFIGURATION{0x07};
constexpr uint8_t LED_CONFIGURATION{0x09};
// TEMPERATURE
constexpr uint8_t READ_TEMP_INTEGER{0x16};
constexpr uint8_t READ_TEMP_FRACTION{0x17};
// PART ID
constexpr uint8_t REVISION_ID{0xFE};
constexpr uint8_t PART_ID{0xFF};

}  // namespace command
}  // namespace max30100
///@endcond

}  // namespace unit
}  // namespace m5

#endif