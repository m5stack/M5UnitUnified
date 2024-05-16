/*!
  @file unit_QMP6988.hpp
  @brief QMP6988 Unit for M5UnitUnified

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UNIT_ENV_UNIT_QMP6988_HPP
#define M5_UNIT_ENV_UNIT_QMP6988_HPP

#include <M5UnitComponent.hpp>

namespace m5 {
namespace unit {

namespace qmp6988 {
/*!
  @enum Average
  @brief Average times setting for oversampling
 */
enum class Average : uint8_t {
    Skip,   //!< @brief No measurement
    Avg1,   //!< @brief 1 time
    Avg2,   //!< @brief 2 times
    Avg4,   //!< @brief 4 times
    Avg8,   //!< @brief 8 times
    Avg16,  //!< @brief 16 times
    Avg32,  //!< @brief 32 times
    Avg64,  //!< @brief 64 times
};

/*!
  @enum PowerMode
  @brief Operation mode setting
 */
enum class PowerMode : uint8_t {
    Sleep =
        0,  //!< @brief Minimal power consumption, but no measurements are taken
    Force =
        1,  //!< @brief Energise the circuit for measurement only when measuring
    Normal = 3,  //!< @brief Normally energized
};

enum class Filter : uint8_t {
    Off,      //!< @brief Off filter
    Coeff2,   //!< @brief co-efficient 2
    Coeff4,   //!< @brief co-efficient 4
    Coeff8,   //!< @brief co-efficient 8
    Coeff16,  //!< @brief co-efficient 16
    Coeff32,  //!< @brief co-efficient 32
};

/*!
  @struct Status
  @brief Accessor for Status
 */
struct Status {
    //! @brief Device operation status. T
    inline bool measure() const {
        return value & (1U << 3);
    }
    // @brief the status of OTP data access
    inline bool OTP() const {
        return value & (1U << 0);
    }
    uint8_t value{};
};

/*!
  @enum StandbyTime
  @brief Standby time setting for power mode Normal
  @detail Measuerment interval
 */
enum class StandbyTime {
    Time1ms,    //!< @brief 1 ms
    Time5ms,    //!< @brief 5 ms
    Time50ms,   //!< @brief 50 ms
    Time250ms,  //!< @brief 250 ms
    Time500ms,  //!< @brief 500 ms
    Time1sec,   //!< @brief 1 seconds
    Time2sec,   //!< @brief 2 seconds
    Time4sec,   //!< @brief 4 seconds
};

struct Calibration {
    int32_t b00{}, bt1{}, bp1{};
    int64_t bt2{};
    int32_t b11{}, bp2{}, b12{}, b21{}, bp3{}, a0{}, a1{}, a2{};
};

};  // namespace qmp6988

/*!
  @class UnitQMP6988
  @brief Barometric pressure sensor to measure atmospheric pressure and altitude
  estimation
*/
class UnitQMP6988 : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x70};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitQMP6988(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitQMP6988() {
    }

    virtual bool begin() override;
    virtual void update() override;

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
    inline float pressure() const {
        return _pressure;
    }
    ///@}

    // API
    ///@name Typical use case setup
    ///@{
    /*! @brief For weather monitoring */
    inline bool setWeathermonitoring() {
        return setMeasurementCondition(qmp6988::Average::Avg2,
                                       qmp6988::Average::Avg1) &&
               setFilterCoeff(qmp6988::Filter::Off);
    }
    //! @brief For drop detection
    bool setDropDetection() {
        return setMeasurementCondition(qmp6988::Average::Avg4,
                                       qmp6988::Average::Avg1) &&
               setFilterCoeff(qmp6988::Filter::Off);
    }
    //! @brief For elevator detection
    bool setElevatorDetection() {
        return setMeasurementCondition(qmp6988::Average::Avg8,
                                       qmp6988::Average::Avg1) &&
               setFilterCoeff(qmp6988::Filter::Coeff4);
    }
    //! @brief For stair detection
    bool setStairDetection() {
        return setMeasurementCondition(qmp6988::Average::Avg16,
                                       qmp6988::Average::Avg2) &&
               setFilterCoeff(qmp6988::Filter::Coeff8);
    }
    //! @brief For indoor navigation
    bool setIndoorNavigation() {
        return setMeasurementCondition(qmp6988::Average::Avg32,
                                       qmp6988::Average::Avg4) &&
               setFilterCoeff(qmp6988::Filter::Coeff32);
    }
    ///@}

    ///@name Measurement condition
    ///@{
    bool getMeasurementCondition(qmp6988::Average& ta, qmp6988::Average& pa,
                                 qmp6988::PowerMode& mode);
    bool setMeasurementCondition(const qmp6988::Average ta,
                                 const qmp6988::Average pa,
                                 const qmp6988::PowerMode mode);
    bool setMeasurementCondition(const qmp6988::Average ta,
                                 const qmp6988::Average pa);
    bool setMeasurementCondition(const qmp6988::Average ta);

    inline bool setTemperatureOversampling(const qmp6988::Average a) {
        return setMeasurementCondition(a);
    }
    bool setPressureOversampling(const qmp6988::Average a);
    bool setPowerMode(const qmp6988::PowerMode mode);
    ///@}

    ///@name IIR filter co-efficient setting
    ///@{
    bool getFilterCoeff(qmp6988::Filter& f);
    bool setFilterCoeff(const qmp6988::Filter& f);
    ///@}

    ///@name Standby time setting for power mode normal
    ///@{
    bool getStandbyTime(qmp6988::StandbyTime& st);
    bool setStandbyTime(const qmp6988::StandbyTime st);
    ///@}

    bool readMeasurement();

    bool reset();
    bool softReset();
    bool getStatus(qmp6988::Status& s);

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

    bool get_measurement_condition(uint8_t& cond);
    bool set_measurement_condition(const uint8_t cond);
    bool get_io_setup(uint8_t& s);
    bool set_io_setup(const uint8_t s);
    bool wait_measurement();
    bool read_calibration(qmp6988::Calibration& c);

   protected:
    bool _periodic{};  // During periodic measurement?
    bool _updated{};
    unsigned long _latest{}, _interval{};
    float _temperature{}, _pressure{};

    qmp6988::Average _tempAvg{qmp6988::Average::Skip};
    qmp6988::Average _pressureAvg{qmp6988::Average::Skip};
    qmp6988::PowerMode _mode{qmp6988::PowerMode::Sleep};
    qmp6988::Calibration _calibration{};
};

namespace qmp6988 {
namespace command {

constexpr uint8_t CHIP_ID{0xD1};

constexpr uint8_t READ_TEMPERATURE{0xFA};  // ~ FC 3bytes
constexpr uint8_t READ_PRESSURE{0xF7};     // ~ F9 3bytes

constexpr uint8_t IO_SETUP{0xF5};
constexpr uint8_t CONTROL_MEASUREMENT{0xF4};
constexpr uint8_t GET_STATUS{0xF3};
constexpr uint8_t IIR_FILTER{0xF1};

constexpr uint8_t RESET{0xE0};

constexpr uint8_t READ_COMPENSATION_COEFFICIENT{0xA0};  // ~ 0xB8 25 butes

}  // namespace command
}  // namespace qmp6988

}  // namespace unit
}  // namespace m5
#endif
