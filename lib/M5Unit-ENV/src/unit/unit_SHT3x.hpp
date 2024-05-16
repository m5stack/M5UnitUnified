/*!
  @file unit_SHT3x.hpp
  @brief SHT3x fmalily Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UNIT_ENV_UNIT_SHT3x_HPP
#define M5_UNIT_ENV_UNIT_SHT3x_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

namespace sht3x {
/*!
  @enum Repeatability
  @brief Repeatability accuracy level
 */
enum class Repeatability : uint8_t {
    High,    //!< @brief High repeatability
    Medium,  //!< @brief Medium repeatability
    Low      //!< @brief Low repeatability
};

/*!
  @enum MPS
  @brief Measuring frequency
 */
enum class MPS : uint8_t {
    MpsHalf,  //!< @brief 0.5 measurement per second
    Mps1,     //!< @brief 1 measurement per second
    Mps2,     //!< @brief 2 measurement per second
    Mps10,    //!< @brief 10 measurement per second
};

/*!
  @struct Status
  @brief Accessor for Status
  @note The order of the bit fields cannot be controlled, so bitwise
  operations are used to obtain each value.
  @note Items marked with (*) are subject to clear status
 */
struct Status {
    //! @brief Alert pending status (*)
    inline bool alertPending() const {
        return value & (1U << 15);
    }
    //! @brief Heater status
    inline bool heater() const {
        return value & (1U << 13);
    }
    //! @brief RH tracking alert (*)
    inline bool trackingAlertRH() const {
        return value & (1U << 11);
    }
    //! @brief Tracking alert (*)
    inline bool trackingAlert() const {
        return value & (1U << 10);
    }
    //! @brief System reset detected (*)
    inline bool reset() const {
        return value & (1U << 4);
    }
    //! @brief Command staus
    inline bool command() const {
        return value & (1U << 1);
    }
    //! @brief Write data checksum status
    inline bool checksum() const {
        return value & (1U << 0);
    }

    uint16_t value{};
};
}  // namespace sht3x

/*!
  @class UnitSHT30
  @brief Temperature and humidity, sensor unit
*/
class UnitSHT30 : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x44};
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
        //! @brief Measuring frequency if start periodic on begin
        sht3x::MPS mps{sht3x::MPS::Mps1};
        //! @brief Repeatability accuracy level if start periodic on begin
        sht3x::Repeatability rep{sht3x::Repeatability::High};
        //! @brief start heater on begin?
        bool start_heater{false};
    };

    explicit UnitSHT30(const uint8_t addr = DEFAULT_ADDRESS) : Component(addr) {
    }
    virtual ~UnitSHT30() {
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
    inline unsigned long updatedMilliss() const {
        return _latest;
    }
    //! @brief Latest neasured temperature (Celsius)
    inline float temperature() const {
        return _temperature;
    }
    //! @brief Latest measured humidity (RH)
    inline float humidity() const {
        return _humidity;
    }
    ///@}

    // API
    ///@name Single shot measurement
    ///@{
    /*!
      @brief Measurement single shot
      @param rep Repeatability accuracy level
      @param stretch Enable clock stretching if true
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool measurementSingleShot(
        const sht3x::Repeatability rep = sht3x::Repeatability::High,
        const bool stretch             = true);
    ///@}

    ///@name Periodic
    ///@{
    /*!
      @brief Start periodic measurement
      @param[in] mps Measurement per second
      @param[in] rep Repeatability accuracy level
      @return True if successful
    */
    bool startPeriodicMeasurement(
        const sht3x::MPS mps           = sht3x::MPS::Mps1,
        const sht3x::Repeatability rep = sht3x::Repeatability::High);
    /*!
      @brief Stop periodic measurement
      @return True if successful
    */
    bool stopPeriodicMeasurement();
    /*!
      @brief Check for fresh data and store
      @return True if fresh data is available
      @warning Only available during periodic measurements.
    */
    bool readMeasurement();
    /*!
      @brief set ART mode
      @details After issuing the ART command the sensor will start acquiring
      data with a frequency of 4Hz.
      @return True if successful
      @warning Only available during periodic measurements.
    */
    bool accelerateResponseTime();
    ///@}

    ///@name Reset
    ///@{
    /*!
      @brief Soft reset
      @details The sensor to reset its system controller and reloads calibration
      data from the memory.
      @return True if successful
      @warning During periodic detection runs, an error is returned
    */
    bool softReset();
#if 0
    /*!
      @brief General reset
      @details Reset using I2C general call
      @warning All devices on the same I2C bus that support the general call
      mode will perform a reset
      @return True if successful
     */
    bool generalReset();
#endif
    ///@}

    ///@name Heater
    ///@{
    /*!
      @brief Start heater
      @return True if successful
     */
    bool startHeater();
    /*!
      @brief Stop heater
      @return True if successful
     */
    bool stopHeater();
    ///@}

    ///@name Status
    ///@{
    /*!
      @brief Get status
      @param[out] s Status
      @return True if successful
    */
    bool getStatus(sht3x::Status& s);
    /*!
      @brief Clear status
      @note @sa Status
      @return True if successful
    */
    bool clearStatus();
    ///@}

    ///@name Serial
    ///@{
    /*!
      @brief Get the serial number value
      @param[out] serialNumber serial number value
      @return True if successful
      @note The serial number is 32 bit
      @warning During periodic detection runs, an error is returned
    */
    bool getSerialNumber(uint32_t& serialNumber);
    /*!
      @brief Get the serial number string
      @param[out] serialNumber Output buffer
      @return True if successful
      @warning Size must be at least 9 bytes
      @warning During periodic detection runs, an error is returned
    */
    bool getSerialNumber(char* serialNumber);
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
    bool read_measurement();

   protected:
    bool _periodic{};  // During periodic measurement?
    bool _updated{};
    unsigned long _latest{}, _interval{};

    // latest data
    float _temperature{};  // C
    float _humidity{};     // RH

    config_t _cfg{};
};

///@cond
namespace sht3x {
namespace command {
// Measurement Commands for Single Shot Data Acquisition Mode
constexpr uint16_t SINGLE_SHOT_ENABLE_STRETCH_HIGH{0x2C06};
constexpr uint16_t SINGLE_SHOT_ENABLE_STRETCH_MEDIUM{0x2C0D};
constexpr uint16_t SINGLE_SHOT_ENABLE_STRETCH_LOW{0x2C10};
constexpr uint16_t SINGLE_SHOT_DISABLE_STRETCH_HIGH{0x2400};
constexpr uint16_t SINGLE_SHOT_DISABLE_STRETCH_MEDIUM{0x240B};
constexpr uint16_t SINGLE_SHOT_DISABLE_STRETCH_LOW{0x2416};
// Measurement Commands for Periodic Data Acquisition Mode
constexpr uint16_t START_PERIODIC_MPS_HALF_HIGHT{0x2032};
constexpr uint16_t START_PERIODIC_MPS_HALF_MEDIUM{0x2024};
constexpr uint16_t START_PERIODIC_MPS_HALF_LOW{0x202f};
constexpr uint16_t START_PERIODIC_MPS_1_HIGH{0x2130};
constexpr uint16_t START_PERIODIC_MPS_1_MEDIUM{0x2126};
constexpr uint16_t START_PERIODIC_MPS_1_LOW{0x212D};
constexpr uint16_t START_PERIODIC_MPS_2_HIGH{0x2236};
constexpr uint16_t START_PERIODIC_MPS_2_MEDIUM{0x2220};
constexpr uint16_t START_PERIODIC_MPS_2_LOW{0x222B};
constexpr uint16_t START_PERIODIC_MPS_10_HIGH{0x2737};
constexpr uint16_t START_PERIODIC_MPS_10_MEDIUM{0x2721};
constexpr uint16_t START_PERIODIC_MPS_10_LOW{0x272A};
constexpr uint16_t STOP_PERIODIC_MEASUREMENT{0x3093};
constexpr uint16_t ACCELERATED_RESPONSE_TIME{0x2B32};
constexpr uint16_t READ_MEASUREMENT{0xE000};
// Reset
constexpr uint16_t SOFT_RESET{0x30A2};
// Heater
constexpr uint16_t START_HEATER{0x306D};
constexpr uint16_t STOPE_HEATER{0x3066};
// Status
constexpr uint16_t READ_STATUS{0xF32D};
constexpr uint16_t CLEAR_STATUS{0x3041};
// Serial
constexpr uint16_t GET_SEREAL_NUMBER_ENABLE_STRETCH{0x3780};
constexpr uint16_t GET_SEREAL_NUMBER_DISABLE_STRETCH{0x3780};
}  // namespace command
}  // namespace sht3x
///@endcond

}  // namespace unit
}  // namespace m5
#endif
