/*!
  @file M5UnitComponent.hpp
  @brief Main header of M5UnitComponent

  @mainpage M5UnitComponent
  Library for componentising the functions of each device so that they can be
  handled by M5UnitUnified.
  C++11 or later

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
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
  @brief For unit component system
*/
namespace unit {

/*!
  @class Component
  @brief Abstract base class of unit component
 */
class Component {
   public:
    /*!
      @struct component_config_t
      @brief UnitComponent basic settings
     */
    struct component_config_t {
        //! @brief Does the user call Unit's update? (default as false)
        bool self_update{};
        //! @brief Maximum number of units that can be connected (default as 0)
        uint8_t max_children{};
    };

    ///@name Fixed parameters for class
    ///@warning Define the same name and type in the derived class.
    ///@{
    static const types::uid_t uid;    //!< @brief Unique identifier for device
    static const types::attr_t attr;  //!< @brief Attributes
    static const char name[];         //!< @brief device name
    ///@}

    ///@name Constructor
    ///@warning COPY PROHIBITED
    ///@{
    explicit Component(const uint8_t addr = 0x00);
    Component(const Component&) = delete;
    Component(Component&&);
    ///@}

    ///@name Assignment
    ///@warning COPY PROHIBITED
    ///@{
    Component& operator=(const Component&) = delete;
    Component& operator=(Component&&);
    ///@}

    virtual ~Component() = default;

    ///@name Settings
    ///@{
    /*! @brief Gets the configuration */
    component_config_t component_config() {
        return _uccfg;
    }
    //! @brief Set the configuration
    void component_config(const component_config_t& cfg) {
        _uccfg = cfg;
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
    Adapter* getAdapter(const uint8_t ch) {
        return ensure_adapter(ch);
    }
    ///@}

    ///@name Bus assignment
    ///@{
    /*! @brief Assgin m5::hal::bus */
    bool assign(m5::hal::bus::Bus* bus);
    /*! @brief Assgin TwoWire */
    //    [[deprecated("use assign(m5::hal::bus::Bus* bus);")]]
    bool assign(TwoWire& wire);
    ///@}

    ///@name Functions that must be inherited
    ///@{
    /*! @brief Begin unit */
    virtual bool begin() {
        return true;
    }
    //! @brief Update unit
    virtual void update() {
    }
    ///@}

    ///@name R/W
    ///@{
    /*! @brief Reading data with transactions */
    m5::hal::error::error_t readWithTransaction(uint8_t* data,
                                                const size_t len);
    //! @brief Read data from register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister(const Reg reg, uint8_t* rbuf, const size_t len,
                      const uint32_t delayMillis, const bool stop = true);
    //! @brief Read uint8_t from register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister8(const Reg reg, uint8_t& result,
                       const uint32_t delayMillis, const bool stop = true);
    //! @brief Read uint16_t from register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister16(const Reg reg, uint16_t& result,
                        const uint32_t delayMillis, const bool stop = true);

    //! @brief Writeing data with transactions */
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

    //! @brief Write data to register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister(const Reg reg, const uint8_t* buf = nullptr,
                       const size_t len = 0U, const bool stop = true);
    //! @brief Write uint8_t to register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister8(const Reg reg, const uint8_t value,
                        const bool stop = true);
    //! @brief Write uint16_t to register
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value &&
                                          std::is_unsigned<Reg>::value &&
                                          sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister16(const Reg reg, const uint16_t value,
                         const bool stop = true);
    ///@}

    ///@name Children
    ///@brief For daisy-chaining units such as hubs
    ///@{
    /*! @brief It means whether the device is connected to a device other than
     * the main unit. */
    bool hasParent() const {
        return _parent;
    }
    //! @brief Are there any other devices connected to the same unit besides
    //! yourself?
    bool hasSiblings() const {
        return _prev || _next;
    }
    //! @brief Are there other devices connected to you?
    bool hasChildren() const {
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

    ///@name Iterator for children
    ///@{
    ///@cond
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
    ///@endcond
    child_iterator childBegin() noexcept {
        return child_iterator(_child);
    }
    child_iterator childEnd() noexcept {
        return child_iterator(nullptr);
    }
    const_child_iterator childBegin() const noexcept {
        return const_child_iterator(_child);
    }
    const_child_iterator childEnd() const noexcept {
        return const_child_iterator(nullptr);
    }
    ///@}

    //! @brief Output information for debug
    virtual std::string debugInfo() const;

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

   protected:
    UnitUnified* _manager{};
    std::unique_ptr<m5::unit::Adapter> _adapter{};

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

}  // namespace unit
}  // namespace m5
#endif
