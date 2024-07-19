/*!
  @file unit_SGP30.hpp
  @brief SGP30 Unit for M5UnitUnified

  @mainpage M5UnitTVOC/eCO2

  TVOC/eCO2 mini Unit is a digital multi-pixel gas sensor unit with integrated
  SGP30.

  It mainly measures various VOC (volatile organic compounds) and H2 in the air.
  It can be programmed to detect TVOC (total volatile organic compounds) and
  eCO2 (equivalent carbon dioxide reading)Concentration measurement.

  Typical measurement accuracy is 15% within the measurement range, the SGP30
  reading is internally calibrated and output, which can maintain long-term
  stability. SGP30 uses I2C protocol communication with on-chip humidity
  compensation function, which can be turned on through an external humidity
  sensor.

  If you need to obtain accurate results, you need to calibrate according to a
  known measurement source. SGP30 has a built-in calibration function. In
  addition, eCO2 is calculated based on the concentration of H2 and cannot
  completely replace "true" CO2 sensors for laboratory use.

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_TVOC_UNIT_SGP30_HPP
#define M5_UNIT_TVOC_UNIT_SGP30_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

/*!
  @namespace sgp30
  @brief For SGP30
 */
namespace sgp30 {
/*!
  @struct Feature
  @brief Structure of the SGP feature set number
 */
struct Feature {
    //! @brief product type (SGP30: 0)
    uint8_t productType() const {
        return (value >> 12) & 0x0F;
    }
    /*!
      @brief product version
      @note Please note that the last 5 bits of the productversion (bits 12-16
      of the LSB) are subject to change
      @note This is used to track new features added tothe SGP multi-pixel
      platform
     */
    uint8_t productVersion() const {
        return value & 0xFF;
    }
    uint16_t value{};
};

}  // namespace sgp30

/*!
  @class UnitSGP30
  @brief SGP30 unit
 */
class UnitSGP30 : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitSGP30, 0x58);

   public:
    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
        //! @brief Start periodic measurement on begin()?
        bool start_periodic{true};
        //! @brief  Baseline CO2eq initial value
        uint16_t baseline_co2eq{};
        //! @brief  Baseline TVOC initial value
        uint16_t baseline_tvoc{};
        //! @brief Absolute humidity initiali value
        uint16_t humidity{};
        /*!
          @brief Inceptive Baseline for TVOC measurements initial value
          @warning The application of this feature is solely limited to the very
          first start-up period of an SGP sensor
         */
        uint16_t inceptive_tvoc{};
    };

    explicit UnitSGP30(const uint8_t addr = DEFAULT_ADDRESS) : Component(addr) {
    }
    virtual ~UnitSGP30() {
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
    /*!
      @brief Gets the product version
      @warning Calling after the call of begin()
     */
    inline uint8_t productVersion() const {
        return _version;
    }
    //! @brief Baseline data updated?
    inline bool updatedBaseline() const {
        return _updatedBaseline;
    }
    /*!
      @brief Time elapsed since start-up when the baseline data was updated
      in update()
      @return Updated time (Unit: ms)
    */
    inline unsigned long updatedBaselineMillis() const {
        return _latestBaseline;
    }
    //! @brief Latest CO2eq (ppm)
    inline uint16_t co2eq() const {
        return _CO2eq;
    }
    //! @brief Latest TVOC (ppb)
    inline uint16_t tvoc() const {
        return _TVOC;
    }
    /*!
      @brief Latest baseline CO2eq
      @note Updates every hour
    */
    inline uint16_t basellineCo2eq() const {
        return _baselineCO2eq;
    }
    /*!
      @brief Latest baseline TVOC
      @note Updates every hour
     */
    inline uint16_t baselineTvoc() const {
        return _baselineTVOC;
    }
    ///@}

    ///@name Measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @param co2eq iaq baseline for CO2eq
      @param tvoc iaq baseline for TVOC
      @param humidity absolute humidity
      @return True if successful
      @note 15 seconds wait is required before a valid measurement can be
      initiated
      @note In update(), waiting are taken into account
      @warning Measurement duration max 10 ms
    */
    bool startPeriodicMeasurement(const uint16_t co2eq, const uint16_t tvoc,
                                  const uint16_t humidity);
    /*!
      @brief Start periodic measurement
      @return True if successful
      @note 15 seconds wait is required before a valid measurement can be
      initiated<br>
      In update(), waiting are taken into account
      @warning Measurement duration max 10 ms
    */
    inline bool startPeriodicMeasurement() {
        return start_periodic_measurement();
    }
    /*!
      @brief Stop periodic measurement
      @return True if successful
    */
    bool stopPeriodicMeasurement();
    /*!
      @brief Various measurements and store values
      @param[out] co2eq
      @param[out] tvoc
      @return True if successful
      @note When periodic measurements are taken, they are called in update
      and the values are stored.
      @warning Measurement duration max 12 ms
    */
    bool readMeasurement(uint16_t& co2eq, uint16_t& tvoc);
    /*!
      @brief Read sensor raw signals
      @param[out] h2
      @param[out] etoh
      @return True if successful
      @warning Measurement duration max 25 ms
    */
    bool readRaw(uint16_t& h2, uint16_t& etoh);
    ///@}

    ///@name Correction
    ///@{
    /*!
      @brief Gets the IAQ baseline
      @param[out] co2eq iaq baseline for CO2
      @param[out] tvoc iaq baseline for TVOC
      @return True if successful
      @warning Measurement duration max 10 ms
    */
    bool getIaqBaseline(uint16_t& co2eq, uint16_t& tvoc);
    /*!
      @brief Set the absolute humidity
      @param raw absolute humidity (disable if zero)
      @return True if successful
      @note Value is a fixed-point 8.8bit number
      @warning Measurement duration max 10 ms
    */
    bool setAbsoluteHumidity(const uint16_t raw);
    /*!
      @brief Set the absolute humidity
      @param gm3 absolute humidity (g/m^3)
      @return True if successful
      @warning Measurement duration max 10 ms
    */
    bool setAbsoluteHumidity(const float gm3);

    /*!
      @brief Gets the inceptive Basebine for TVOC
      @param[out] tvoc Inceptive baseline
      @return True if successful
      @warning Measurement duration max 10 ms
      @warning Only available if product version is 0x21 or higher
    */
    bool getTvocInceptiveBaseline(uint16_t& tvoc);
    /*!
      @brief Sets the inceptive Basebine for TVOC
      @param tvoc Inceptive baseline
      @return True if successful
      @warning Measurement duration max 10 ms
      @warning The application of this feature is solely limited to the very
      first start-up period of an SGP sensor
      @warning Only available if product version is 0x21 or higher
    */
    bool setTvocInceptiveBaseline(const uint16_t tvoc);
    ///@}

    /*!
      @brief Run the on-chip self-test
      @param[out] result self-test return code
      @return True if successful
      @note If the test is OK, the result will be 0xd400
      @warning Measurement duration max 220 ms
      @warning Must not be executed after startPeriodicMeasurement
    */
    bool measureTest(uint16_t& result);
    /*!
      @brief General reset
      @details Reset using I2C general call
      @warning This is a reset by General command, the command is also sent
      to all devices with I2C connections
      @return True if successful
    */
    bool generalReset();
    /*!
      @brief Gets the feature set
      @param feature
      @return True if successful
      @warning Measurement duration max 10 ms
     */
    bool getFeatureSet(sgp30::Feature& feature);
    /*!
      @brief Gets the serial number
      @return True if successful
      @note Serial number is 48bits
    */
    bool getSerialNumber(uint64_t& number);
    /*!
      @brief Get the serial number string
      @param[out] number Output buffer
      @return True if successful
      @warning number must be at least 13 bytes
    */
    bool getSerialNumber(char* number);

   protected:
    bool start_periodic_measurement();
    bool set_iaq_baseline(const uint16_t co2eq, const uint16_t tvoc);

   protected:
    uint8_t _version{};  // Chip version
    bool _updatedBaseline{};
    types::elapsed_time_t _latestBaseline{};

    uint16_t _CO2eq{}, _TVOC{};
    uint16_t _baselineCO2eq{}, _baselineTVOC{};

    config_t _cfg{};
};

namespace sgp30 {
namespace command {
///@cond
constexpr uint16_t IAQ_INIT{0x2003};
constexpr uint16_t MEASURE_IAQ{0x2008};
constexpr uint16_t GET_IAQ_BASELINE{0x2015};
constexpr uint16_t SET_IAQ_BASELINE{0x201E};
constexpr uint16_t SET_ABSOLUTE_HUMIDITY{0x2061};
constexpr uint16_t MEASURE_TEST{0x2032};
constexpr uint16_t GET_FEATURE_SET{0x202F};
constexpr uint16_t MEASURE_RAW{0x2050};
constexpr uint16_t GET_TVOC_INCEPTIVE_BASELINE{0x20B3};
constexpr uint16_t SET_TVOC_INCEPTIVE_BASELINE{0x2077};
constexpr uint16_t GET_SERIAL_ID{0x3682};
///@endcond
}  // namespace command
}  // namespace sgp30

}  // namespace unit
}  // namespace m5
#endif
