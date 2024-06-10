/*!
  @file unit_ADS111x.hpp
  @brief ADS111x Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_METER_UNIT_ADS111X_HPP
#define M5_UNIT_METER_UNIT_ADS111X_HPP

#include <M5UnitComponent.hpp>
#include <m5_utility/stl/extension.hpp>

namespace m5 {
namespace unit {

namespace ads111x {

/*!
  @enum Mux
  @brief Input multiplexer
  @warning This feature serve nofunction on the ADS1113 and ADS1114
*/
enum class Mux : uint8_t {
    AIN_01,  //!< @brief Positive:AIN0 Negative:AIN1 as default
    AIN_03,  //!< @brief Positive:AIN0 Negative:AIN3
    AIN_13,  //!< @brief Positive:AIN1 Negative:AIN3
    AIN_23,  //!< @brief Positive:AIN2 Negative:AIN3
    GND_0,   //!< @brief Positive:AIN0 Negative:AIN3
    GND_1,   //!< @brief Positive:AIN1 Negative:GND
    GND_2,   //!< @brief Positive:AIN2 Negative:GND
    GND_3,   //!< @brief Positive:AIN3 Negative:GND
};

/*!
  @enum FSR
  @brief Programmable gain amplifier
  @warning This feature serve nofunction on the ADS1113
 */
enum class Gain : uint8_t {
    PGA_6144,  //!< @brief +/- 6.144 V
    PGA_4096,  //!< @brief +/- 4.096 V
    PGA_2048,  //!< @brief +/- 2.048 V as default
    PGA_1024,  //!< @brief +/- 1.024 V
    PGA_512,   //!< @brief +/- 0.512 V
    PGA_256,   //!< @brief +/- 0.256 V
    // 6,7 PGA_256 (duplicate)
};

/*!
  @enum Rate
  @brief Data rate setting (samples per second)
 */
enum class Rate : uint8_t {
    SPS_8,    //!< @brief 8 sps
    SPS_16,   //!< @brief 16 sps
    SPS_32,   //!< @brief 32 sps
    SPS_64,   //!< @brief 64 sps
    SPS_128,  //!< @brief 128 sps as default
    SPS_250,  //!< @brief 250 sps
    SPS_475,  //!< @brief 475 sps
    SPS_860,  //!< @brief 860 sps
};

/*!
  @enum ComparatorQueue
  @brief the value determines the number of successive conversions exceeding the
  upper orlower threshold required
  @warning This feature serve nofunction on the ADS1113
*/
enum class ComparatorQueue : uint8_t {
    One,      //!< @brief Assert after one conversion
    Two,      //!< @brief Assert after one conversion
    Four,     //!< @brief Assert after one conversion
    Disable,  //!< @brief Disable comparator and set ALERT/RDY pin to
              //!< high-impedance as default
};

/*!
  @struct Config
  @brief Accessor for configration
 */
struct Config {
    ///@name Getter
    ///@{
    /*! @brief Operational status */
    inline bool os() const {
        return value & (1U << 15);
    }
    /*!
      @brief Input multiplexer
      @warning This feature serve nofunction on the ADS1113 and ADS1114
    */
    inline Mux mux() const {
        return static_cast<Mux>((value >> 12) & 0x07);
    }
    /*!
      @brief Programmable gain amplifier
      @warning This feature serve nofunction on the ADS1113
    */
    inline Gain pga() const {
        return static_cast<Gain>((value >> 9) & 0x07);
    }
    //! @brief Device operating mode
    inline bool mode() const {
        return value & (1U << 8);
    }
    //! @brief Data rate
    inline Rate dr() const {
        return static_cast<Rate>((value >> 5) & 0x07);
    }
    /*!
      @brief Comparator mode
      @warning This feature serve nofunction on the ADS1113
    */
    inline bool comp_mode() const {
        return value & (1U << 4);
    }
    /*!
      @brief Comparator polarity
      @warning This feature serve nofunction on the ADS1113
    */
    inline bool comp_pol() const {
        return value & (1U << 3);
    }
    /*!
      @brief Latching comparator
      @warning This feature serve nofunction on the ADS1113
    */
    inline bool comp_lat() const {
        return value & (1U << 2);
    }
    /*!
      @brief Comparator queue
      @warning This feature serve nofunction on the ADS1113
    */
    inline ComparatorQueue comp_que() const {
        return static_cast<ComparatorQueue>(value & 0x03);
    }
    ///@}

    ///@name Setter
    ///@{
    inline void os(const bool b) {
        value = (value & ~(1U << 15)) | ((b ? 1U : 0) << 15);
    }
    inline void mux(const Mux m) {
        value = (value & ~(0x07 << 12)) |
                ((m5::stl::to_underlying(m) & 0x07) << 12);
    }
    inline void pga(const Gain g) {
        value =
            (value & ~(0x07 << 9)) | ((m5::stl::to_underlying(g) & 0x07) << 9);
    }
    inline void mode(const bool b) {
        value = (value & ~(1U << 8)) | ((b ? 1U : 0) << 8);
    }
    inline void dr(const Rate r) {
        value =
            (value & ~(0x07 << 5)) | ((m5::stl::to_underlying(r) & 0x07) << 5);
    }
    inline void comp_mode(const bool b) {
        value = (value & ~(1U << 4)) | ((b ? 1U : 0) << 4);
    }
    inline void comp_pol(const bool b) {
        value = (value & ~(1U << 3)) | ((b ? 1U : 0) << 3);
    }
    inline void comp_lat(const bool b) {
        value = (value & ~(1U << 2)) | ((b ? 1U : 0) << 2);
    }
    inline void comp_que(const ComparatorQueue c) {
        value = (value & ~0x03U) | (m5::stl::to_underlying(c) & 0x03);
    }
    ///@}

    uint16_t value{};
};

}  // namespace ads111x

/*!
  @class UnitADS111x
  @brief Base class for ADS111x series
 */
class UnitADS111x : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
        bool periodic{true};
        ads111x::Rate rate{ads111x::Rate::SPS_128};
        // The following items are not supported by some classes
        ads111x::Mux mux{ads111x::Mux::AIN_01};
        ads111x::Gain gain{ads111x::Gain::PGA_2048};
        ads111x::ComparatorQueue comp_que{ads111x::ComparatorQueue::Disable};
    };

    explicit UnitADS111x(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitADS111x() {
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
        return !_adsCfg.mode();
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
    //! @brief Gets the latest periodic measurements
    int16_t latestData() const {
        return _value;
    }
    //! @breif Coefficient value
    float coefficient() const {
        return _coefficient;
    }
    //! @breief Resolution of 1 LSB
    virtual float resolution() const {
        return coefficient();
    }
    ///@}

    ///@name Configration
    ///@warning ADS1113, ADS1114 and ADS1115 differ in the items that can be set
    ///@{
    /*! @brief Gets the input multiplexer */
    inline ads111x::Mux multiplexer() const {
        return _adsCfg.mux();
    }
    //! @brief Gets the programmable gain amplifier
    ads111x::Gain gain() const;
    //! @brief Gets the data rate
    inline ads111x::Rate rate() const {
        return _adsCfg.dr();
    }
    /*!
      @brief Gets the comparator mode
      @retval true Window comparator
      @retval false Traditional comparator
     */
    inline bool comparatorMode() const {
        return _adsCfg.comp_mode();
    }
    /*!
      @brief Gets the comparator polarity
      @retval true Active high
      @retval false Active low
     */
    inline bool comparatorPolarity() const {
        return _adsCfg.comp_pol();
    }
    /*!
      @brief Gets the Latching comparator
      @retval true Latching comparator
      @retval false Nonlatching comparator
    */
    inline bool latchingComparator() const {
        return _adsCfg.comp_lat();
    }
    //! @brief Gets the comparator queue
    inline ads111x::ComparatorQueue comparatorQueue() const {
        return _adsCfg.comp_que();
    }

    //! @brief Set the input multiplexer
    virtual bool setMultiplexer(const ads111x::Mux mux) = 0;
    /*!
      @brief Set the programmable gain amplifier
      @warning the threshould values  must be updated whenever the PGA settings
      are changed (@sa setThreshould)
     */
    virtual bool setGain(const ads111x::Gain gain) = 0;
    /*! @brief Set the data rate  */
    bool setRate(const ads111x::Rate rate);
    //! @brief Set the comparator mode
    virtual bool setComparatorMode(const bool b) = 0;
    //! @brief Set the comparator polarity
    virtual bool setComparatorPolarity(const bool b) = 0;
    //! @brief Set the latching comparator
    virtual bool setLatchingComparator(const bool b) = 0;
    //! @brief Set the comparator queue
    virtual bool setComparatorQueue(const ads111x::ComparatorQueue c) = 0;
    ///@}

    ///@name Measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @return True if successful
      @note Frequencies etc. must already be set
     */
    bool startPeriodicMeasurement();
    /*!
      @brief Stop periodic measurement
      @return True if successful
    */
    bool stopPeriodicMeasurement();
    /*!
      @brief Start single measurement
      @return True if successful
      @note After calling this function, the value can be retrieved after the
      conversion has been completed
      @warning Not measured periodic
     */
    bool startSingleMeasurement();
    //! @brief In conversion?
    bool inConversion();  // TODO: to const
    /*!
      @brief Get raw value
      @param raw[out] Raw value
      @return True if successful
      @warning Not under conversion
     */
    bool getAdcRaw(int16_t& raw);
    /*!
      @brief Read value using single measurement
      @param[out] raw Raw value
      @param timeoutMillis Timeout time (Unit: ms)
      @return True if successful
      @note Helper function summarising measurement start~waiting for
      conversion~getting value
      @warning Not measured periodic
     */
    bool readSingleMeasurement(int16_t& raw, const uint32_t timeoutMillis = 10);
    ///@}

    ///@name Threshold
    ///@{
    /*!
      @brief Gets the threshould values
      @param[out] high upper thresould value
      @param[out] low lower thresould value
      @return True if successful
    */
    bool getThreshould(int16_t& high, int16_t& low);
    /*!
      @brief Set the threshould values
      @param high upper thresould value
      @param low lower thresould value
      @return True if successful
      @warning The high value must always be greater than the low value
    */
    bool setThreshould(const int16_t high, const int16_t low);
    ///@}

    /*!
      @brief General reset
      @details Reset using I2C general call
      @warning This is a reset by General command, the command is also sent to
      all devices with I2C connections
     */
    bool generalReset();

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
    inline virtual bool on_begin() {
        return true;
    }

    bool get_config(ads111x::Config& c);
    bool write_config(const ads111x::Config& c);
    void apply_interval(const ads111x::Rate rate);
    void apply_coefficient(const ads111x::Gain gain);

    bool set_multiplexer(const ads111x::Mux mux);
    bool set_gain(const ads111x::Gain gain);
    bool set_comparator_mode(const bool b);
    bool set_comparator_polarity(const bool b);
    bool set_latching_comparator(const bool b);
    bool set_comparator_queue(const ads111x::ComparatorQueue c);

    bool read_ads_raw(int16_t& raw);

   protected:
    bool _updated{};
    unsigned long _latest{}, _interval{};

    int16_t _value{};  // Latest periodic measurements
    float _coefficient{};
    ads111x::Config _adsCfg{};

    config_t _cfg{};
};

/*!
  @class  UnitADS1113
  @brief ADS1113 unit
 */
class UnitADS1113 : public UnitADS111x {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitADS1113(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitADS111x(addr) {
    }
    virtual ~UnitADS1113() {
    }

    ///@name Configration
    ///@{
    /*! @brief Not support @warning Not support */
    virtual bool setMultiplexer(const ads111x::Mux) override {
        return false;
    }
    //!  @brief Not support @warning Not support
    virtual bool setGain(const ads111x::Gain) override {
        return false;
    }
    //!  @brief Not support @warning Not support
    virtual bool setComparatorMode(const bool) override {
        return false;
    }
    //!  @brief Not support @warning Not support
    virtual bool setComparatorPolarity(const bool) override {
        return false;
    }
    //!  @brief Not support @warning Not support
    virtual bool setLatchingComparator(const bool) override {
        return false;
    }
    //!  @brief Not support @warning Not support
    virtual bool setComparatorQueue(const ads111x::ComparatorQueue) override {
        return false;
    }
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
    virtual bool on_begin() override;
};

/*!
  @class  UnitADS1114
  @brief ADS1114 unit
 */
class UnitADS1114 : public UnitADS111x {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitADS1114(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitADS111x(addr) {
    }
    virtual ~UnitADS1114() {
    }

    ///@name Configration
    ///@{
    /*!  @brief Not support @warning Not support */
    virtual bool setMultiplexer(const ads111x::Mux) override {
        return false;
    }
    //! @brief Set the programmable gain amplifier
    virtual bool setGain(const ads111x::Gain gain) override {
        return set_gain(gain);
    }
    //! @brief Set the comparator mode
    virtual bool setComparatorMode(const bool b) override {
        return set_comparator_mode(b);
    }
    //! @brief Set the comparator polarity
    virtual bool setComparatorPolarity(const bool b) override {
        return set_comparator_polarity(b);
    }
    //! @brief Set the latching comparator
    virtual bool setLatchingComparator(const bool b) override {
        return set_latching_comparator(b);
    }
    //! @brief Set the comparator queue
    virtual bool setComparatorQueue(const ads111x::ComparatorQueue c) override {
        return set_comparator_queue(c);
    }
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
    virtual bool on_begin() override;
};

/*!
  @class  UnitADS1115
  @brief ADS1115 unit
 */
class UnitADS1115 : public UnitADS111x {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

    explicit UnitADS1115(const uint8_t addr = DEFAULT_ADDRESS)
        : UnitADS111x(addr) {
    }
    virtual ~UnitADS1115() {
    }

    ///@name Configration
    ///@{
    /*! @brief Set the input multiplexer */
    virtual bool setMultiplexer(const ads111x::Mux mux) override {
        return set_multiplexer(mux);
    }
    //! @brief Set the programmable gain amplifier
    virtual bool setGain(const ads111x::Gain gain) override {
        return set_gain(gain);
    }
    //! @brief Set the comparator mode
    virtual bool setComparatorMode(const bool b) override {
        return set_comparator_mode(b);
    }
    //! @brief Set the comparator polarity
    virtual bool setComparatorPolarity(const bool b) override {
        return set_comparator_polarity(b);
    }
    //! @brief Set the latching comparator
    virtual bool setLatchingComparator(const bool b) override {
        return set_latching_comparator(b);
    }
    //! @brief Set the comparator queue
    virtual bool setComparatorQueue(const ads111x::ComparatorQueue c) override {
        return set_comparator_queue(c);
    }
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
    virtual bool on_begin() override;
};

///@cond
namespace ads111x {
namespace command {

constexpr uint8_t CONVERSION_REG{0x00};
constexpr uint8_t CONFIG_REG{0x01};
constexpr uint8_t LOW_THRESHOLD_REG{0x02};
constexpr uint8_t HIGH_THRESHOLD_REG{0x03};

}  // namespace command
}  // namespace ads111x
///@endcond

}  // namespace unit
}  // namespace m5

#endif
