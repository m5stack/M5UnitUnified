/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_SCD4x.hpp
  @brief SCD4X family Unit for M5UnitUnified
*/
#ifndef M5_UNIT_ENV_UNIT_SCD4x_HPP
#define M5_UNIT_ENV_UNIT_SCD4x_HPP

#include <M5UnitComponent.hpp>
#include <m5_utility/container/circular_buffer.hpp>
#include <limits>  // NaN

namespace m5 {
namespace unit {

/*!
  @namespace scd4x
  @brief For SCD4X family
 */
namespace scd4x {
/*!
  @enum Mode
  @brief Mode of periodic measurement
 */
enum class Mode : uint8_t {
    Normal,    //!< Normal (Receive data every 5 seconds)
    LowPower,  //!< Low power (Receive data every 30 seconds)
};

///@cond 0
// Max command duration(ms)
constexpr uint16_t READ_MEASUREMENT_DURATION{1};
constexpr uint16_t STOP_PERIODIC_MEASUREMENT_DURATION{500};
constexpr uint16_t SET_TEMPERATURE_OFFSET_DURATION{1};
constexpr uint16_t GET_TEMPERATURE_OFFSET_DURATION{1};
constexpr uint16_t SET_SENSOR_ALTITUDE_DURATION{1};
constexpr uint16_t GET_SENSOR_ALTITUDE_DURATION{1};
constexpr uint16_t SET_AMBIENT_PRESSURE_DURATION{1};
constexpr uint16_t PERFORM_FORCED_CALIBRATION_DURATION{400};
constexpr uint16_t SET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION{1};
constexpr uint16_t GET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION{1};
constexpr uint16_t GET_DATA_READY_STATUS_DURATION{1};
constexpr uint16_t PERSIST_SETTINGS_DURATION{800};
constexpr uint16_t GET_SERIAL_NUMBER_DURATION{1};
constexpr uint16_t PERFORM_SELF_TEST_DURATION{10000};
constexpr uint16_t PERFORM_FACTORY_RESET_DURATION{1200};
constexpr uint16_t REINIT_DURATION{20};
constexpr uint16_t MEASURE_SINGLE_SHOT_DURATION{5000};
constexpr uint16_t MEASURE_SINGLE_SHOT_RHT_ONLY_DURATION{50};
///@endcond

}  // namespace scd4x

/*!
  @class UnitSCD40
  @brief CO2 sensor unit
*/
class UnitSCD40 : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitSCD40, 0x62);

   public:
    /*!
      @struct config_t
      @brief Settings for begin
     */
    struct config_t : Component::config_t {
        //! Start periodic measurement on begin?
        bool start_periodic{true};
        //! Mode of periodic measurement if start on begin?
        scd4x::Mode mode{scd4x::Mode::Normal};
        //! Enable calibration on begin?
        bool calibration{true};
    };

    /*!
      @struct Data
      @brief Measurement data group
     */
    struct Data {
        std::array<uint8_t, 9> raw{};  //!< RAW data
        uint16_t co2() const;          //!< CO2 concentration (ppm)
        float temperature() const;     //!< temperature (Celsius)
        float humidity() const;        //!< humidity (RH)
    };

    explicit UnitSCD40(const uint8_t addr = DEFAULT_ADDRESS) : Component(addr) {
        _data.reset(new m5::container::CircularBuffer<Data>(1));
    }
    virtual ~UnitSCD40() {
    }

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

    ///@name Settings for begin
    ///@{
    /*! @brief Gets the configration */
    config_t config() const {
        return _cfg;
    }
    //! @brief Set the configration
    void config(const config_t &cfg) {
        _cfg = cfg;
    }
    ///@}

    ///@name Measurement data by periodic
    ///@{
    //! @brief Latest measured CO2 concentration (ppm)
    inline uint16_t co2() const {
        return !_data->empty() ? _data->back()->co2() : 0;
    }
    //! @brief Latest measured temperature (Celsius)
    inline float temperature() const {
        return !_data->empty() ? _data->back()->temperature()
                               : std::numeric_limits<float>::quiet_NaN();
    }
    //! @brief Latest measured humidity (RH)
    inline float humidity() const {
        return !_data->empty() ? _data->back()->humidity()
                               : std::numeric_limits<float>::quiet_NaN();
    }
    //! @brief Get the number of stored data
    inline size_t available() const {
        return _data->size();
    }
    //! @brief Empty data?
    inline bool empty() const {
        return _data->empty();
    }
    //! @brief Stored data full?
    inline bool full() const {
        return _data->full();
    }
    //! @brief Retrieve oldest stored data
    inline Data oldest() const {
        return !_data->empty() ? *(_data->front()) : Data{};
    }
    //! @brief Retrieve latest stored data
    inline Data latest() const {
        return !_data->empty() ? *(_data->back()) : Data{};
    }
    //! @brief Discard  the oldest data accumulated
    inline void discard() const {
        _data->pop_front();
    }
    //! @brief Discard all data
    inline void flush() {
        _data->clear();
    }
    ///@}

    ///@name Periodic
    ///@{
    /*!
      @brief Start periodic measurement
      @param mode Mode of periodic measurement
      @return True if successful
    */
    bool startPeriodicMeasurement(const scd4x::Mode mode = scd4x::Mode::Normal);
    /*!
      @brief Start low power periodic measurements
      @return True if successful
    */
    inline bool startLowPowerPeriodicMeasurement() {
        return startPeriodicMeasurement(scd4x::Mode::LowPower);
    }
    /*!
      @brief Stop measurement
      @param duration Max command duration(ms)
      @warning The sensor will only respond to other commands after waiting 500
      ms after issuing the stop_periodic_measurement command
      @return True if successful
    */
    bool stopPeriodicMeasurement(
        const uint32_t duration = scd4x::STOP_PERIODIC_MEASUREMENT_DURATION);
    ///@}

    ///@name On-Chip Output Signal Compensation
    ///@{
    /*!
      @brief Set the temperature offset
      @details Define how warm the sensor is compared to ambient, so RH and T
      are temperature compensated. Has no effect on the CO2 reading Default
      offsetis 4C
      @param offset (0 <= offset < 175)
      @param duration Max command duration(ms)
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool setTemperatureOffset(
        const float offset,
        const uint32_t duration = scd4x::SET_TEMPERATURE_OFFSET_DURATION);
    /*!
      @brief Read the temperature offset
      @param[out] offset Offset value
      @param duration Max command duration(ms)
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool readTemperatureOffset(float &offset);
    /*!
      @brief Set the sensor altitude
      @details Define the sensor altitude in metres above sea level, so RH and
      CO2 arecompensated for atmospheric pressure Default altitude is 0m
      @param altitude Unit:metres
      @param duration Max command duration(ms)
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool setSensorAltitude(
        const uint16_t altitude,
        const uint32_t duration = scd4x::SET_SENSOR_ALTITUDE_DURATION);
    /*!
      @brief Read the sensor altitude
      @param[out] altitude Altitude value
      @param duration Max command duration(ms)
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool readSensorAltitude(uint16_t &altitude);
    /*!
      @brief Set the ambient pressure
      @details Define the ambient pressure in Pascals, so RH and CO2 are
      compensated for atmospheric pressure setAmbientPressure overrides
      setSensorAltitude
      @param presure Unit: pascals (>= 0.0f)
      @param duration Max command duration(ms)
      @return True if successful
    */
    bool setAmbientPressure(
        const float pressure,
        const uint32_t duration = scd4x::SET_AMBIENT_PRESSURE_DURATION);
    ///@}

    ///@name Field Calibration
    ///@{
    /*!
      @brief Perform forced recalibration
      @param concentration Unit:ppm
      @param[out] correction The FRC correction value
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 400 ms
    */
    bool performForcedRecalibration(const uint16_t concentration,
                                    int16_t &correction);
    /*!
      @brief Enable/disable automatic self calibration
      @param enabled Enable automatic self calibration if true
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool setAutomaticSelfCalibrationEnabled(
        const bool enabled = true,
        const uint32_t duration =
            scd4x::SET_AUTOMATIC_SELF_CALIBRATION_ENABLED_DURATION);
    /*!
      @brief Check if automatic self calibration is enabled
      @param[out] enabled  True if automatic self calibration is enabled
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool readAutomaticSelfCalibrationEnabled(bool &enabled);
    ///@}

    ///@name Advanced Features
    ///@{
    /*!
      @brief Copy sensor settings from RAM to EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool persistSettings(
        const uint32_t duration = scd4x::PERSIST_SETTINGS_DURATION);
    /*!
      @brief Get the serial number string
      @param[out] serialNumber Output buffer
      @return True if successful
      @warning Size must be at least 13 bytes
      @warning During periodic detection runs, an error is returned
    */
    bool readSerialNumber(char *serialNumber);
    /*!
      @brief Get the serial number value
      @param[out] serialNumber serial number value
      @return True if successful
      @note The serial number is 48 bit
      @warning During periodic detection runs, an error is returned
    */
    bool readSerialNumber(uint64_t &serialNumber);
    /*!
      @brief Perform self test
      @param[out] True if malfunction detected
      @return True if successful
      @note Takes 10 seconds to complete
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 10000 ms
    */
    bool performSelfTest(bool &malfunction);
    /*!
      @brief Peform factory reset
      @details Reset all settings to the factory values
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1200 ms
    */
    bool performFactoryReset(
        const uint32_t duration = scd4x::PERFORM_FACTORY_RESET_DURATION);
    /*!
      @brief Re-initialize the sensor, load settings from EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 20 ms
    */
    bool reInit(const uint32_t duration = scd4x::REINIT_DURATION);
    ///@}

   protected:
    bool read_data_ready_status();
    bool read_measurement(Data &d, const bool all = true);

   protected:
    std::unique_ptr<m5::container::CircularBuffer<Data>> _data{};
    config_t _cfg{};
};

/*!
  @class UnitSCD41
  @brief CO2 sensor unit
*/
class UnitSCD41 : public UnitSCD40 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitSCD41, 0x00);

   public:
    explicit UnitSCD41(const uint8_t addr = DEFAULT_ADDRESS) : UnitSCD40(addr) {
    }

    ///@name Low power single shot (SCD41)
    ///@{
    /*!
      @brief Request a single measurement
      @return True if successful
      @note Values are updated at 5000 ms interval
      @warning During periodic detection runs, an error is returned
    */
    bool measureSingleshot(UnitSCD40::Data &d);
    /*!
      @brief Request a single measurement temperature and humidity
      @return True if successful
      @note Values are updated at 50 ms interval
      @warning Information on CO2 is invalid.
      @warning During periodic detection runs, an error is returned
    */
    bool measureSingleshotRHT(UnitSCD40::Data &d);
    ///@}
};

///@cond
namespace scd4x {
namespace command {
// Basic Commands
constexpr uint16_t START_PERIODIC_MEASUREMENT{0x21b1};
constexpr uint16_t READ_MEASUREMENT{0xec05};
constexpr uint16_t STOP_PERIODIC_MEASUREMENT{0x3f86};

// On-chip output signal compensation
constexpr uint16_t SET_TEMPERATURE_OFFSET{0x241d};
constexpr uint16_t GET_TEMPERATURE_OFFSET{0x2318};
constexpr uint16_t SET_SENSOR_ALTITUDE{0x2427};
constexpr uint16_t GET_SENSOR_ALTITUDE{0x2322};
constexpr uint16_t SET_AMBIENT_PRESSURE{0xe000};

// Field calibration
constexpr uint16_t PERFORM_FORCED_CALIBRATION{0x362f};
constexpr uint16_t SET_AUTOMATIC_SELF_CALIBRATION_ENABLED{0x2416};
constexpr uint16_t GET_AUTOMATIC_SELF_CALIBRATION_ENABLED{0x2313};

// Low power
constexpr uint16_t START_LOW_POWER_PERIODIC_MEASUREMENT{0x21ac};
constexpr uint16_t GET_DATA_READY_STATUS{0xe4b8};

// Advanced features
constexpr uint16_t PERSIST_SETTINGS{0x3615};
constexpr uint16_t GET_SERIAL_NUMBER{0x3682};
constexpr uint16_t PERFORM_SELF_TEST{0x3639};
constexpr uint16_t PERFORM_FACTORY_RESET{0x3632};
constexpr uint16_t REINIT{0x3646};

// Low power single shot - SCD41 only
constexpr uint16_t MEASURE_SINGLE_SHOT{0x219d};
constexpr uint16_t MEASURE_SINGLE_SHOT_RHT_ONLY{0x2196};

}  // namespace command
}  // namespace scd4x
///@endcond

}  // namespace unit
}  // namespace m5
#endif
