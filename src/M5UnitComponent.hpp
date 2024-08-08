/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitComponent.hpp
  @brief Main header of M5UnitComponent

  @mainpage M5UnitComponent
  Library for componentising the functions of each device so that they can be
  handled by M5UnitUnified. C++11 or later.
*/
#ifndef M5_UNIT_COMPONENT_HPP
#define M5_UNIT_COMPONENT_HPP

#include "m5_unit_component/types.hpp"
#include "m5_unit_component/adapter.hpp"
#include "m5_unit_component/utility.hpp"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <memory>

class TwoWire;

namespace m5 {
namespace unit {
class UnitUnified;
class Adapter;
}  // namespace unit
}  // namespace m5

/*!
  @namespace m5
  @brief Top level namespace of M5
 */
namespace m5 {

/*!
  @namespace unit
  @brief For unit components
*/
namespace unit {

/*!
  @class Component
  @brief Base class of unit component
 */
class Component {
   public:
    /*!
      @struct component_config_t
      @brief Component basic settings
     */
    struct component_config_t {
        //! Clock for communication (default as 100000)
        uint32_t clock{100000};
        //! Does the user call Unit's update? (default as false)
        bool self_update{false};
        //! Maximum number of units that can be connected (default as 0)
        uint8_t max_children{0};
    };

    /*!
      @struct config_t
      @brief Base settings for begin
      @note Derived classes are defined by deriving their own config_t from this
     */
    struct config_t {
        //! Maximum number of periodic measurement data to be stored
        uint32_t stored_size{1};
    };

    ///@warning Define the same name and type in the derived class.
    ///@name Fixed parameters for class
    ///@{
    static const types::uid_t uid;    //!< @brief Unique identifier for device
    static const types::attr_t attr;  //!< @brief Attributes
    static const char name[];         //!< @brief device name
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Constructor
    ///@{
    explicit Component(const uint8_t addr = 0x00);
    Component(const Component&)     = delete;
    Component(Component&&) noexcept = default;
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Assignment
    ///@{
    Component& operator=(const Component&)     = delete;
    Component& operator=(Component&&) noexcept = default;
    ///@}

    virtual ~Component() = default;

    ///@name Settings
    ///@{
    /*! @brief Gets the configuration */
    inline component_config_t component_config() {
        return _uccfg;
    }
    //! @brief Set the configuration
    inline void component_config(const component_config_t& cfg) {
        _uccfg = cfg;
    }

    ///@}

    ///@name Functions that must be inherited
    ///@{
    /*!
      @brief Begin unit
      @warning Call parent begin in inherited function
    */
    virtual bool begin() {
        return true;
    }
    /*!
      @brief Update unit
      @param force Forced communication for updates if true
    */
    virtual void update(const bool force = false) {
        (void)force;
    }
    ///@}

    ///@name Properties
    ///@{
    /*!  @brief Gets the device name */
    inline const char* deviceName() const {
        return unit_device_name();
    }
    //! @brief Gets the identifier
    inline types::uid_t identifier() const {
        return unit_identifier();
    }
    //! @brief Gets the attributes
    inline types::attr_t attribute() const {
        return unit_attribute();
    }
    //! @brief Gets the registered order (== 0 means not yet)
    inline uint32_t order() const {
        return _order;
    }
    //! @brief Gets the channel if connected to another unit
    inline int16_t channel() const {
        return _channel;
    }
    //! @brief Is registered to Units manager?
    inline bool isRegistered() const {
        return _manager != nullptr;
    }
    //! @brief Address used to access the device
    inline uint8_t address() const {
        return _addr;
    }
    //! @brief Gets the adapter for children
    inline Adapter* getAdapter(const uint8_t ch) {
        return ensure_adapter(ch);
    }
    ///@}

    ///@name Periodic measurement
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
    inline types::elapsed_time_t updatedMillis() const {
        return _latest;
    }
    /*!
      @brief Gets the periodic measurement interval
      @return interval time (Unit: ms)
     */
    inline types::elapsed_time_t interval() const {
        return _interval;
    }
    ///@}

    ///@name Bus assignment
    ///@{
    /*! @brief Assgin m5::hal::bus */
    virtual bool assign(m5::hal::bus::Bus* bus);
    /*! @brief Assgin TwoWire */
    virtual bool assign(TwoWire& wire);
    ///@}

    ///@note For daisy-chaining units such as hubs
    ///@name Parent-children relationship
    ///@{
    /*! @brief Has parent unit? */
    inline bool hasParent() const {
        return _parent;
    }
    //! @brief Are there any other devices connected to the same parent unit
    //! besides yourself?
    inline bool hasSiblings() const {
        return _prev || _next;
    }
    //! @brief Are there other devices connected to me?
    inline bool hasChildren() const {
        return _child;
    }
    //! @brief Number of units connected to me
    size_t childrenSize() const;
    //! @brief Is there a unit connected to the specified channel?
    bool exists(const uint8_t ch) const;
    //! @brief Gets the deviceconnected to the specified channel
    Component* child(const uint8_t chhanle) const;
    //! @brief Connect the unit to the specified channel
    bool add(Component& c, const int16_t channel);
    //! @brief Select valid channel if exists
    bool selectChannel(const uint8_t ch = 8);
    ///@}

    template <class T>
    class Iterator : public std::iterator<std::forward_iterator_tag, T*> {
       public:
        explicit Iterator(T* c) : _cur(c) {
        }
        Iterator& operator++() {
            if (_cur) {
                _cur = _cur->_next;
            }
            return *this;
        }
        inline T& operator*() const {
            return *_cur;
        }
        inline T* operator->() const {
            return _cur;
        }
        inline bool operator==(const Iterator& o) const {
            return _cur == o._cur;
        }
        inline bool operator!=(const Iterator& o) const {
            return !operator==(o);
        }

       private:
        T* _cur{};
    };
    using child_iterator       = Iterator<Component>;
    using const_child_iterator = Iterator<const Component>;

    ///@name Iterator for children
    ///@{
    inline child_iterator childBegin() noexcept {
        return child_iterator(_child);
    }
    inline child_iterator childEnd() noexcept {
        return child_iterator(nullptr);
    }
    inline const_child_iterator childBegin() const noexcept {
        return const_child_iterator(_child);
    }
    inline const_child_iterator childEnd() const noexcept {
        return const_child_iterator(nullptr);
    }
    ///@}

    /*! @brief General call for I2C*/
    bool generalCall(const uint8_t* data, const size_t len);

    //! @brief Output information for debug
    virtual std::string debugInfo() const;

    ///@name Read/Write
    ///@{
    m5::hal::error::error_t readWithTransaction(uint8_t* data,
                                                const size_t len);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister(const Reg reg, uint8_t* rbuf, const size_t len,
                      const uint32_t delayMillis, const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister8(const Reg reg, uint8_t& result,
                       const uint32_t delayMillis, const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister16(const Reg reg, uint16_t& result,
                        const uint32_t delayMillis, const bool stop = true);
    m5::hal::error::error_t writeWithTransaction(const uint8_t* data,
                                                 const size_t len,
                                                 const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    m5::hal::error::error_t writeWithTransaction(const Reg reg,
                                                 const uint8_t* data,
                                                 const size_t len,
                                                 const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister(const Reg reg, const uint8_t* buf = nullptr,
                       const size_t len = 0U, const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister8(const Reg reg, const uint8_t value,
                        const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister16(const Reg reg, const uint16_t value,
                         const bool stop = true);
    ///@}

   protected:
    // Proper implementation in derived classes is required
    virtual const char* unit_device_name() const = 0;
    virtual types::uid_t unit_identifier() const = 0;
    virtual types::attr_t unit_attribute() const = 0;

    // Ensure the adapter for children if Hub
    virtual Adapter* ensure_adapter(const uint8_t /*ch*/) {
        return nullptr;
    }
    // Select valid channel if exists(Hub etc...)
    virtual m5::hal::error::error_t select_channel(const uint8_t) {
        return m5::hal::error::error_t::OK;
    }

    bool add_child(Component* c);
    // Functions for dynamically addressable devices
    bool changeAddress(const uint8_t addr);

   protected:
    UnitUnified* _manager{};
    std::unique_ptr<m5::unit::Adapter> _adapter{};

    // For periodic measurement
    types::elapsed_time_t _latest{}, _interval{};
    bool _periodic{};  // During periodic measurement?
    bool _updated{};

   private:
    uint32_t _order{};
    component_config_t _uccfg{};
    int16_t _channel{-1};  // valid [0...]
    uint8_t _addr{};
    bool _begun{};

    // for chain
    Component* _parent{};
    Component* _next{};
    Component* _prev{};
    Component* _child{};

    friend class UnitUnified;
};

/*!
  @class PeriodicMeasurementAdapter
  @brief Interface class for periodic measurement
  @details Common interface for accumulated periodic measurement data
  @details Provide a common interface for periodic measurements for each unit
  @tparam Derived Derived class
  @tparam MD Type of the measurement data group
  @warning MUST IMPLEMENT some functions (NOT VERTUAL)
  - MD Derived::oldest_periodic_data() const;
  - MD Derived::latestt_periodic_data() const;
  - bool Derived::start_periodic_measurement(any arguments);
  - bool Derived::stop_periodic_measurement():
  @warning  MUST ADD std::unique_ptr<m5::container::CircularBuffer<MD>> _data{}
  in Derived class
  @warning This class is an interface class and should not have any data
  @note See also M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER
  @note
*/
template <class Derived, typename MD>
class PeriodicMeasurementAdapter {
   public:
    ///@name Periodic measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @tparam Args Optional arguments
      @return True if successful
      @note Call Derived::start_periodic_measurement
    */
    template <typename... Args>
    bool startPeriodicMeasurement(Args&&... args) {
        return static_cast<Derived*>(this)->start_periodic_measurement(
            std::forward<Args>(args)...);
    }
    /*!
      @brief Stop periodic measurement
      @tparam Args Optional arguments
      @return True if successful
      @note Call Derived::stop_periodic_measurement
    */
    template <typename... Args>
    bool stopPeriodicMeasurement(Args&&... args) {
        return static_cast<Derived*>(this)->stop_periodic_measurement(
            std::forward<Args>(args)...);
    }
    ///@}

    ///@name Data
    ///@{
    //! @brief Gets the number of stored data
    inline size_t available() const {
        return available_periodic_measurement_data();
    }
    //! @brief Is empty stored data?
    inline bool empty() const {
        return empty_periodic_measurement_data();
    }
    //! @brief Is stored data full?
    inline bool full() const {
        return full_periodic_measurement_data();
    }
    //! @brief Retrieve oldest stored data
    inline MD oldest() const {
        return static_cast<const Derived*>(this)->oldest_periodic_data();
    }
    //! @brief Retrieve latest stored data
    inline MD latest() const {
        return static_cast<const Derived*>(this)->latest_periodic_data();
    }
    //! @brief Discard  the oldest data accumulated
    inline void discard() {
        discard_periodic_measurement_data();
    }
    //! @brief Discard all data
    inline void flush() {
        flush_periodic_measurement_data();
    }
    ///@}

  protected:
    ///@note Must implement in derived class
    ///@name Pure virtual functions
    ///@{
    virtual size_t available_periodic_measurement_data() const = 0;
    virtual bool empty_periodic_measurement_data() const       = 0;
    virtual bool full_periodic_measurement_data() const        = 0;
    virtual void discard_periodic_measurement_data()           = 0;
    virtual void flush_periodic_measurement_data()             = 0;
    ///@}
};

}  // namespace unit
}  // namespace m5

// Helper for creating derived classes from Component
///@cond
#define M5_UNIT_COMPONENT_HPP_BUILDER(cls, reg)                    \
   public:                                                         \
    constexpr static uint8_t DEFAULT_ADDRESS{(reg)};               \
    static const types::uid_t uid;                                 \
    static const types::attr_t attr;                               \
    static const char name[];                                      \
                                                                   \
    cls(const cls&)                = delete;                       \
    cls& operator=(const cls&)     = delete;                       \
    cls(cls&&) noexcept            = default;                      \
    cls& operator=(cls&&) noexcept = default;                      \
                                                                   \
   protected:                                                      \
    inline virtual const char* unit_device_name() const override { \
        return name;                                               \
    }                                                              \
    inline virtual types::uid_t unit_identifier() const override { \
        return uid;                                                \
    }                                                              \
    inline virtual types::attr_t unit_attribute() const override { \
        return attr;                                               \
    }

// Helper for creating derived class from PeriodicMeasurementAdapter
#define M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(cls, md) \
   protected:                                                               \
    friend class PeriodicMeasurementAdapter<cls, md>;                       \
                                                                            \
    inline md oldest_periodic_data() const {                                \
        return !_data->empty() ? _data->front().value() : md{};             \
    }                                                                       \
    inline md latest_periodic_data() const {                                \
        return !_data->empty() ? _data->back().value() : md{};              \
    }                                                                       \
    inline virtual size_t available_periodic_measurement_data()             \
        const override {                                                    \
        return _data->size();                                               \
    }                                                                       \
    inline virtual bool empty_periodic_measurement_data() const override {  \
        return _data->empty();                                              \
    }                                                                       \
    inline virtual bool full_periodic_measurement_data() const override {   \
        return _data->full();                                               \
    }                                                                       \
    inline virtual void discard_periodic_measurement_data() override {      \
        _data->pop_front();                                                 \
    }                                                                       \
    inline virtual void flush_periodic_measurement_data() override {        \
        _data->clear();                                                     \
    }

///@endcond
#endif