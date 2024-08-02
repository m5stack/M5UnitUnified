/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_MAX30100.hpp
  @brief MAX30100 Unit for M5UnitUnified
*/
#ifndef M5_UNIT_HEART_UNIT_MAX30100_HPP
#define M5_UNIT_HEART_UNIT_MAX30100_HPP

#include <M5UnitComponent.hpp>
#include "m5_utility/stl/extension.hpp"
#include "m5_utility/container/circular_buffer.hpp"
#include <limits>  // NaN

namespace m5 {
namespace unit {
/*!
  @namespace max30100
  @brief For MAX30100
 */
namespace max30100 {
/*!
  @enum Mode
  @brief For Mode control
 */
enum class Mode : uint8_t {
    HROnly = 0x02,  //!< HR only enabled
    SPO2,           //!< SPO2 and HR enabled
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
  @enum Sampling
  @brief Sample rate for pulse
  @details Unit is the number of sample per second
 */
enum class Sampling : uint8_t {
    Rate50,    //!< 50 sps
    Rate100,   //!< 100 sps
    Rate167,   //!< 167 sps
    Rate200,   //!< 200 sps
    Rate400,   //!< 400 sps
    Rate600,   //!< 600 sps
    Rate800,   //!< 800 sps
    Rate1000,  //!< 1000 sps
};

/*!
  @enum LedPulseWidth
  @brief  LED pulse width (the IR and RED have the same pulse width)
*/
enum class LedPulseWidth {
    PW200,   //!< 200 us (ADC 13 bits)
    PW400,   //!< 400 us (ADC 14 bits)
    PW800,   //!< 800 us (ADC 15 bits)
    PW1600,  //!< 1600 us (ADC 16 bits)
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
    Sampling samplingRate() const {
        return static_cast<Sampling>((value >> 2) & 0x07);
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
    void samplingRate(const Sampling rate) {
        value = (value & ~(0x07 << 2)) |
                ((m5::stl::to_underlying(rate) & 0x07) << 2);
    }
    void ledPulseWidth(const LedPulseWidth width) {
        value = (value & ~0x03) | (m5::stl::to_underlying(width) & 0x03);
    }
    ///@}

    uint8_t value{};
};

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

//! @brief FIFO depth
constexpr uint8_t MAX_FIFO_DEPTH{16};

/*!
  @struct Data
  @brief Measurement data group
 */
struct Data {
    std::array<uint8_t, 4> raw{};
    uint16_t ir() const;   //!< IR
    uint16_t red() const;  //!< RED
};

/*!
  @struct TemperatureData
  @brief Measurement data group for temperature
 */
struct TemperatureData {
    std::array<uint8_t, 2> raw{};
    //! temperature (Celsius)
    inline float temperature() const {
        return celsius();
    }
    float celsius() const;     //!< temperature (Celsius)
    float fahrenheit() const;  //!< temperature (Fahrenheit)
};

}  // namespace max30100

/*!
  @class UnitMAX30100
  @brief Pulse oximetry and heart-rate sensor
  @note The only single measurement is temperature; other data is constantly
  measured and stored
*/
class UnitMAX30100
    : public Component,
      public PeriodicMeasurementAdapter<UnitMAX30100, max30100::Data> {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitMAX30100, 0x57);

   public:
    /*!
      @struct config_t
      @brief Settings for begin
     */
    struct config_t : Component::config_t {
        //! @brief Operating mode
        max30100::Mode mode{max30100::Mode::HROnly};
        //! @brief Sampling rate
        max30100::Sampling samplingRate{m5::unit::max30100::Sampling::Rate100};
        //! @brief Led pulse width
        max30100::LedPulseWidth pulseWidth{
            m5::unit::max30100::LedPulseWidth::PW1600};
        //! @brief The SpO2 ADC resolution
        bool highResolution{true};
        //! @brief Led current for IR
        m5::unit::max30100::CurrentControl irCurrent{
            //            m5::unit::max30100::CurrentControl::mA7_6};
            m5::unit::max30100::CurrentControl::mA27_1};
        //! @brief Led current for Red
        m5::unit::max30100::CurrentControl redCurrent{
            //            m5::unit::max30100::CurrentControl::mA7_6};
            m5::unit::max30100::CurrentControl::mA27_1};
    };

    explicit UnitMAX30100(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr),
          _data{new m5::container::CircularBuffer<max30100::Data>(
              max30100::MAX_FIFO_DEPTH)} {
        auto cfg        = config();
        cfg.stored_size = max30100::MAX_FIFO_DEPTH;
        config(cfg);

        auto ccfg  = component_config();
        ccfg.clock = 400000U;
        component_config(ccfg);
    }
    virtual ~UnitMAX30100() {
    }

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

    ///@name Settings for begin
    ///@{
    /*! @brief Gets the configration */
    inline config_t config() {
        return _cfg;
    }
    //! @brief Set the configration
    inline void config(const config_t& cfg) {
        _cfg = cfg;
    }
    ///@}

    ///@name Measurement data by periodic
    ///@{
    //! @brief Oldest CO2eq (ppm)
    inline uint16_t ir() const {
        return !empty() ? oldest().ir() : 0;
    }
    //! @brief Oldest TVOC (ppb)
    inline uint16_t red() const {
        return !empty() ? oldest().red() : 0;
    }
    /*!
      @brief Number of data last retrieved
      @note The number of data retrieved by the latest update, not all data
      accumulated
      @sa available()
    */
    inline uint8_t retrived() const {
        return _retrived;
    }
    /*!
      @brief The number of samples lost
      @note It saturates at 15
     */
    inline uint8_t overflow() const {
        return _overflow;
    }
    ///@}

    ///@warning Note that there are different combinations that can be set
    /// depending on the mode See also SpO2Configuration
    ///@name Mode Configuration
    ///@{
    /*!
      @brief Read Mode configuration
      @param[out] mc ModeConfigration
      @return True if successful
     */
    bool readModeConfiguration(max30100::ModeConfiguration& mc);
    /*!
      @brief Set Mode configuration
      @brief read Mode configuration
      @param mc ModeConfigration
      @return True if successful
    */
    bool setModeConfiguration(const max30100::ModeConfiguration mc);
    //! @brief Set Mode
    bool setMode(const max30100::Mode mode);
    //! @brief Enable power save mode
    bool enablePowerSave() {
        return enable_power_save(true);
    }
    //! @brief Disable power save mode
    bool disablePowerSave() {
        return enable_power_save(false);
    }
    ///@}

    ///@warning Note that there are different combinations that can be set
    /// depending on the mode See also SpO2Configuration
    ///@name SpO2 Configuration
    ///@{
    /*!
      @brief Read SpO2 configrartion
      @param[out] sc SpO2Configration
      @return True if successful
    */
    bool readSpO2Configuration(max30100::SpO2Configuration& sc);
    /*!
      @brief Set SpO2 configrartion
      @param sc SpO2Configration
      @return True if successful
    */
    bool setSpO2Configuration(const max30100::SpO2Configuration sc);
    //! @brief Set sampling rate
    bool setSamplingRate(const max30100::Sampling rate);
    //! @brief Set LED pulse width
    bool setLedPulseWidth(const max30100::LedPulseWidth width);
    //! @brief Enable high resolution mode
    inline bool enableHighResolution() {
        return enable_high_resolution(true);
    }
    //! @brief Disable high resolution mode
    inline bool disableHighResolution() {
        return enable_high_resolution(false);
    }
    ///@}

    ///@warning In the heart-rate only mode, the red LED is inactive.
    /// and only the IR LED is used to capture optical data and determine
    /// the heart rate.
    ///@name LED Configuration
    ///@{
    /*!
      @brief Read Led configrartion
      @param[out] lc LedConfigration
      @return True if successful
    */
    bool readLedConfiguration(max30100::LedConfiguration& lc);
    /*!
      @brief Set Led configrartion
      @param lc LedConfigration
      @return True if successful
    */
    bool setLedConfiguration(const max30100::LedConfiguration lc);
    //! @brief Set IR/RED current
    bool setLedCurrent(const max30100::CurrentControl ir,
                       const max30100::CurrentControl red);
    ///@}

    ///@note The temperature sensor data can be used to compensate the SpO2
    /// error with ambient temperature changes
    ///@name Measurement temperature
    ///@{
    /*!
      @brief Measure tempeature single shot
      @param[out] temp Temperature(Celsius)
      @return True if successful
      @warning Blocking until measured about 29 ms
     */
    bool measureTemperatureSingleshot(max30100::TemperatureData& td);
    ///@}

    /*!
      @brief Reset FIFO buffer
      @return True if successful
    */
    bool resetFIFO();
    /*!
      @brief Software reset
      @return True if successful
      @warning Blocked until the reset process is completed
     */
    bool reset();

   protected:
    ///@note Call via startPeriodicMeasurement/stopPeriodicMeasurement
    ///@note MAX30100 is always measured periodic
    ///@name Periodic measurement
    ///@{
    inline bool start_periodic_measurement() {
        return false;
    }
    inline bool stop_periodic_measurement() {
        return false;
    }
    ///@}

    bool read_FIFO();
    bool read_measurement_temperature(max30100::TemperatureData& td);

    M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(UnitMAX30100,
                                                               max30100::Data);

    bool read_mode_configration(uint8_t& c);
    bool set_mode_configration(const uint8_t c);
    bool enable_power_save(const bool enabled);
    bool read_spo2_configration(uint8_t& c);
    bool set_spo2_configration(const uint8_t c);
    bool enable_high_resolution(const bool enabled);
    bool read_led_configration(uint8_t& c);
    bool set_led_configration(const uint8_t c);

    bool read_register(const uint8_t reg, uint8_t* buf, const size_t len);
    bool read_register8(const uint8_t reg, uint8_t& v);

   protected:
    max30100::Mode _mode{};
    max30100::Sampling _samplingRate{};
    uint8_t _retrived{}, _overflow{};
    std::unique_ptr<m5::container::CircularBuffer<max30100::Data>> _data{};

    config_t _cfg{};
};

///@cond
namespace max30100 {
namespace command {
// STATUS
constexpr uint8_t READ_INTERRUPT_STATUS{0x00};
constexpr uint8_t INTERRUPT_ENABLE{0x01};
// FIFO
constexpr uint8_t FIFO_WRITE_POINTER{0x02};
constexpr uint8_t FIFO_OVERFLOW_COUNTER{0x03};
constexpr uint8_t FIFO_READ_POINTER{0x04};
// Note that FIFO_DATA_REGISTER cannot be burst read.
constexpr uint8_t FIFO_DATA_REGISTER{0x05};
// CONFIGURATION
constexpr uint8_t MODE_CONFIGURATION{0x06};
constexpr uint8_t SPO2_CONFIGURATION{0x07};
constexpr uint8_t LED_CONFIGURATION{0x09};
// TEMPERATURE
constexpr uint8_t TEMP_INTEGER{0x16};
constexpr uint8_t TEMP_FRACTION{0x17};
// PART ID
constexpr uint8_t READ_REVISION_ID{0xFE};
constexpr uint8_t PART_ID{0xFF};

}  // namespace command
}  // namespace max30100
///@endcond

}  // namespace unit
}  // namespace m5

#endif
