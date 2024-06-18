/*!
  @file unit_SCD4x.hpp
  @brief SCD4X family Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_ENV_UNIT_SCD4x_HPP
#define M5_UNIT_ENV_UNIT_SCD4x_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

/*!
  @class UnitSCD40
  @brief CO2 sensor unit
*/
class UnitSCD40 : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitSCD40, 0x62);

   public:
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

    explicit UnitSCD40(const uint8_t addr = DEFAULT_ADDRESS) : Component(addr) {
    }
    virtual ~UnitSCD40() {
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
    void config(const config_t &cfg) {
        _cfg = cfg;
    }
    ///@}

    ///@name Properties
    ///@{
    /*! @brief In periodic measurement? */
    inline bool inPeriodic() const {
        return _periodic;
    }
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
    //! @brief Latest measured CO2 concentration (ppm)
    inline uint16_t co2() const {
        return _co2;
    }
    //! @brief Latest measured temperature (Celsius)
    inline float temperature() const {
        return _temperature;
    }
    //! @brief Latest measured humidity (RH)
    inline float humidity() const {
        return _humidity;
    }
    ///@}

    // API
    ///@name Basic Commands
    ///@{
    /*!
      @brief Start measurement
      @note Receive data every 5 seconds
      @return True if successful
    */
    bool startPeriodicMeasurement();
    /*!
      @brief Stop measurement
      @warning The sensor will only respond to other commands after waiting 500
      ms after issuing the stop_periodic_measurement command.
      @return True if successful
    */
    bool stopPeriodicMeasurement();

    /*!
      @brief Check for fresh data and store
      @return True if fresh data is available
      @warning Measurement duration max 1 ms
    */
    bool readMeasurement();
    ///@}

    ///@name On-Chip Output Signal Compensation
    ///@{
    /*!
      @brief Set the temperature offset
      @details Define how warm the sensor is compared to ambient, so RH and T
      are temperature compensated. Has no effect on the CO2 reading Default
      offsetis 4C
      @param[in] offset (0 <= offset < 175)
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool setTemperatureOffset(const float offset);
    /*!
      @brief Get the temperature offset
      @param[out] offset Offset value
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool getTemperatureOffset(float &offset);
    /*!
      @brief Set the sensor altitude
      @details Define the sensor altitude in metres above sea level, so RH and
      CO2 arecompensated for atmospheric pressure Default altitude is 0m
      @param[in] altitude Unit: metres
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool setSensorAltitude(const uint16_t altitude);
    /*!
      @brief Get the sensor altitude
      @param[out] altitude Altitude value
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool getSensorAltitude(uint16_t &altitude);
    /*!
      @brief Set the ambient pressure
      @details Define the ambient pressure in Pascals, so RH and CO2 are
      compensated for atmospheric pressure setAmbientPressure overrides
      setSensorAltitude
      @param[in] presure Unit: pascals (>= 0.0f)
      @return True if successful
      @warning Measurement duration max 1 ms
    */
    bool setAmbientPressure(const float pressure);
    ///@}

    ///@name Field Calibration
    ///@{
    /*!
      @brief Perform forced recalibration
      @param[in] concentration Unit:ppm
      @param[out] correction The FRC correction value
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 400 ms
    */
    bool performForcedRecalibration(const uint16_t concentration,
                                    int16_t &correction);
    /*!
      @brief Enable/disable automatic self calibration
      @param[in] enabled Enable automatic self calibration if true
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool setAutomaticSelfCalibrationEnabled(const bool enabled = true);
    /*!
      @brief Check if automatic self calibration is enabled
      @param[out] enabled  True if automatic self calibration is enabled
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 1 ms
    */
    bool getAutomaticSelfCalibrationEnabled(bool &enabled);
    ///@}

    ///@name Low Power operation
    ///@{
    /*!
      @brief Start low power measurements
      @return True if successful
      @note Receive data every 30 seconds
      @warning During periodic detection runs, an error is returned
    */
    bool startLowPowerPeriodicMeasurement(void);
    /*!
      @brief Is fresh data available?
      @retval true Fresh data is available
      @warning Measurement duration max 1 ms
    */
    bool getDataReadyStatus();
    ///@}

    ///@name Advanced Features
    ///@{
    /*!
      @brief Copy sensor settings from RAM to EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 800 ms
    */
    bool persistSettings();
    /*!
      @brief Get the serial number string
      @param[out] serialNumber Output buffer
      @return True if successful
      @warning Size must be at least 13 bytes
      @warning During periodic detection runs, an error is returned
    */
    bool getSerialNumber(char *serialNumber);
    /*!
      @brief Get the serial number value
      @return Not zero if serial number is valid
      @note The serial number is 48 bit
      @warning During periodic detection runs, an error is returned
    */
    uint64_t getSerialNumber();
    /*!
      @brief Get the serial number value
      @param[out] serialNumber serial number value
      @return True if successful
      @note The serial number is 48 bit
      @warning During periodic detection runs, an error is returned
    */
    bool getSerialNumber(uint64_t &serialNumber);
    /*!
      @brief Perform self test
      @return True if no malfunction detected
      @note Takes 10 seconds to complete
      @warning During periodic detection runs, an error is returned
    */
    bool performSelfTest(void);
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
    bool performFactoryReset();
    /*!
      @brief Re-initialize the sensor, load settings from EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 20 ms
    */
    bool reInit();
    ///@}

   protected:
    bool read_measurement(const bool all = true);

   protected:
    constexpr static unsigned long SIGNAL_INTERVAL_MS{5000};
    constexpr static unsigned long SIGNAL_INTERVAL_LOW_MS{30 * 1000};

    bool _periodic{};  // During periodic measurement?
    bool _updated{};
    unsigned long _latest{}, _interval{SIGNAL_INTERVAL_MS};

    // latest data
    uint16_t _co2{};       // ppm
    float _temperature{};  // C
    float _humidity{};     // RH

    config_t _cfg{};
};

/*!
  @class UnitSCD41
  @brief CO2 sensor unit
*/
class UnitSCD41 : public UnitSCD40 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitSCD41, 0xFF);

   public:
    explicit UnitSCD41(const uint8_t addr = DEFAULT_ADDRESS) : UnitSCD40(addr) {
    }

    // API
    // Has the same API as UnitSCD40 and has a dedicated API
    ///@name Low power single shot (SCD41)
    ///@{
    /*!
      @brief Request a single measurement
      @return True if successful
      @note Data will be ready in 5 seconds
      @note The sensor output is read using the readMeasurement()
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 5000 ms
    */
    bool measureSingleShot(void);
    /*!
      @brief Request temperature and humidity
      @return True if successful
      @note Data will be ready in 50ms
      @note The sensor output is read using the readMeasurement()
      @warning CO2 output is returned as 0 ppm.
      @warning During periodic detection runs, an error is returned
      @warning Measurement duration max 50 ms
    */
    bool measureSingleShotRHTOnly(void);
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
