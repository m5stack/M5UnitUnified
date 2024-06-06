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
    AIN_01,  //!< @brief AIN0 and AINn = AIN1 as default
    AIN_03,  //!< @brief AIN0 and AINn = AIN3
    AIN_13,  //!< @brief AIN1 and AINn = AIN3
    AIN_23,  //!< @brief AIN2 and AINn = AIN3
    GND_0,   //!< @brief AIN0 and AINn = GND
    GND_1,   //!< @brief AIN1 and AINn = GND
    GND_2,   //!< @brief AIN2 and AINn = GND
    GND_3,   //!< @brief AIN3 and AINn = GND
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
      @brief Programmable gain ampli
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

class UnitADS111x : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0xFF};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

#if 0
    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
    };
#endif

    explicit UnitADS111x(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitADS111x() {
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
    void config(const config_t& cfg) {
        _cfg = cfg;
    }
    ///@}
#endif

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

    inline ads111x::Mux mux() const {
        return _adsCfg.mux();
    }
    inline ads111x::Gain gain() const {
        return _adsCfg.pga();
    }
    inline ads111x::Rate rate() const {
        return _adsCfg.dr();
    }
    inline ads111x::ComparatorQueue compQueue() const {
        return _adsCfg.comp_que();
    }
    ///@}

    ///@name Configration
    ///@{
    bool setMultiplexer(const ads111x::Mux mux);
    bool setGain(const ads111x::Gain gain);
    bool setRate(const ads111x::Rate rate);
    bool setComparatorQueue(const ads111x::ComparatorQueue c);
    ///@}

    ///@name Periodic
    ///@{
    bool startPeriodicMeasurement();
    bool stopPeriodicMeasurement();
    ///@}

    ///@name Single shot
    ///@{
    bool startSingleMeasurement();

    bool inConversion();  // TODO: to const
    ///@}

    ///@name Get conversion
    ///@{
    bool getConversion(uint16_t& cv, const uint32_t millis = 100);
    bool getAdcRaw(uint16_t& raw);
    ///@}

    /*!
      @brief General reset
      @details Reset using I2C general call
      @waning This is a reset by General command, the command is also sent to
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

    bool get_config();
    bool apply_config();

   protected:
    bool _periodic{};  // During periodic measurement?
    bool _updated{};
    unsigned long _latest{}, _interval{};

    ads111x::Config _adsCfg{};
};

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
};

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
};

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
};

///@cond
namespace ads111x {
namespace command {

constexpr uint8_t CONVERSION_REG{0x00};
constexpr uint8_t CONFIG_REG{0x01};
constexpr uint8_t LOW_THRESHOLD{0x02};
constexpr uint8_t HIGH_THRESHOLD{0x03};

}  // namespace command
}  // namespace ads111x
///@endcond

}  // namespace unit
}  // namespace m5

#endif
