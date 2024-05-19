/*!
  @file unit_SCD4x.hpp
  @brief SCD4X family Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
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
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x62};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

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
    bool stopPeriodicMeasurement(const uint16_t delayMillis = 500);

    /*!
      @brief Check for fresh data and store
      @return True if fresh data is available
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
    */
    bool setTemperatureOffset(const float offset,
                              const uint16_t delayMillis = 1);
    /*!
      @brief Get the temperature offset
      @return Not zero if offset is valid
      @warning During periodic detection runs, an error is returned
    */
    float getTemperatureOffset(void);
    /*!
      @brief Get the temperature offset
      @param[out] offset Offset value
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool getTemperatureOffset(float &offset);

    /*!
      @brief Set the sensor altitude
      @details Define the sensor altitude in metres above sea level, so RH and
      CO2 arecompensated for atmospheric pressure Default altitude is 0m
      @param[in] altitude Unit: metres
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool setSensorAltitude(const uint16_t altitude,
                           const uint16_t delayMillis = 1);
    /*!
      @brief Get the sensor altitude
      @return Not zero if altitude is valid
      @warning During periodic detection runs, an error is returned
    */
    uint16_t getSensorAltitude(void);
    /*!
      @brief Get the sensor altitude
      @param[out] altitude Altitude value
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool getSensorAltitude(uint16_t &altitude);

    /*!
      @brief Set the ambient pressure
      @details Define the ambient pressure in Pascals, so RH and CO2 are
      compensated for atmospheric pressure setAmbientPressure overrides
      setSensorAltitude
      @param[in] presure Unit: pascals (>= 0.0f)
      @return True if successful
    */
    bool setAmbientPressure(const float pressure,
                            const uint16_t delayMillis = 1);
    ///@}

    ///@name Field Calibration
    ///@{
    /*!
      @brief Perform forced recalibration
      @return The FRC correction value
      @warning During periodic detection runs, an error is returned
    */
    int16_t performForcedRecalibration(const uint16_t concentration);
    /*!
      @brief Perform forced recalibration
      @param[in] concentration Unit:ppm
      @param[out] correction The FRC correction value
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool performForcedRecalibration(const uint16_t concentration,
                                    int16_t &correction);

    /*!
      @brief Enable/disable automatic self calibration
      @param[in] enabled Enable automatic self calibration if true
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool setAutomaticSelfCalibrationEnabled(const bool enabled         = true,
                                            const uint16_t delayMillis = 1);
    /*!
      @brief Check if automatic self calibration is enabled
      @return True if automatic self calibration is enabled
      @warning During periodic detection runs, an error is returned
    */
    bool getAutomaticSelfCalibrationEnabled(void);
    /*!
      @brief Check if automatic self calibration is enabled
      @param[out] enabled  True if automatic self calibration is enabled
      @return True if successful
      @warning During periodic detection runs, an error is returned
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
     */
    bool getDataReadyStatus();
    ///@}

    ///@name Advanced Features
    ///@{
    /*!
      @brief Copy sensor settings from RAM to EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool persistSettings(uint16_t delayMillis = 800);
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
    */
    bool performSelfTest(bool &malfunction);
    /*!
      @brief Peform factory reset
      @details Reset all settings to the factory values
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool performFactoryReset(uint16_t delayMillis = 1200);
    /*!
      @brief Re-initialize the sensor, load settings from EEPROM
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool reInit(uint16_t delayMillis = 20);
    ///@}

   protected:
    inline virtual const char *unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }

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
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x62};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

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
    */
    bool measureSingleShot(void);
    /*!
      @brief Request temperature and humidity
      @return True if successful
      @note Data will be ready in 50ms
      @note The sensor output is read using the readMeasurement()
      @warning CO2 output is returned as 0 ppm.
      @warning During periodic detection runs, an error is returned
    */
    bool measureSingleShotRHTOnly(void);
    ///@}

   protected:
    inline virtual const char *unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }
};

///@cond
namespace scd4x {
namespace command {
// Basic Commands
constexpr uint16_t START_PERIODIC_MEASUREMENT{0x21b1};
constexpr uint16_t READ_MEASUREMENT{0xec05};           // execution time: 1ms
constexpr uint16_t STOP_PERIODIC_MEASUREMENT{0x3f86};  // execution time: 500ms

// On-chip output signal compensation
constexpr uint16_t SET_TEMPERATURE_OFFSET{0x241d};  // execution time: 1ms
constexpr uint16_t GET_TEMPERATURE_OFFSET{0x2318};  // execution time: 1ms
constexpr uint16_t SET_SENSOR_ALTITUDE{0x2427};     // execution time: 1ms
constexpr uint16_t GET_SENSOR_ALTITUDE{0x2322};     // execution time: 1ms
constexpr uint16_t SET_AMBIENT_PRESSURE{0xe000};    // execution time: 1ms

// Field calibration
constexpr uint16_t PERFORM_FORCED_CALIBRATION{0x362f};  // execution time: 400ms
constexpr uint16_t SET_AUTOMATIC_SELF_CALIBRATION_ENABLED{
    0x2416};  // execution time: 1ms
constexpr uint16_t GET_AUTOMATIC_SELF_CALIBRATION_ENABLED{
    0x2313};  // execution time: 1ms

// Low power
constexpr uint16_t START_LOW_POWER_PERIODIC_MEASUREMENT{0x21ac};
constexpr uint16_t GET_DATA_READY_STATUS{0xe4b8};  // execution time: 1ms

// Advanced features
constexpr uint16_t PERSIST_SETTINGS{0x3615};       // execution time: 800ms
constexpr uint16_t GET_SERIAL_NUMBER{0x3682};      // execution time: 1ms
constexpr uint16_t PERFORM_SELF_TEST{0x3639};      // execution time: 10000ms
constexpr uint16_t PERFORM_FACTORY_RESET{0x3632};  // execution time: 1200ms
constexpr uint16_t REINIT{0x3646};                 // execution time: 20ms

// Low power single shot - SCD41 only
constexpr uint16_t MEASURE_SINGLE_SHOT{0x219d};
// execution time: 5000ms
constexpr uint16_t MEASURE_SINGLE_SHOT_RHT_ONLY{0x2196};
// execution time: 50ms

}  // namespace command
}  // namespace scd4x
///@endcond

}  // namespace unit
}  // namespace m5
#endif
