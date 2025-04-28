/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnified.hpp
  @brief Main header of M5UnitUnified

  @mainpage M5UnitUnified
  M5UnitUnified  is a library for unified handling of various M5 unit products.
  - Unified APIs
  - Unified Connections
  - Unified Licensing
*/
#ifndef M5_UNIT_UNIFIED_HPP
#define M5_UNIT_UNIFIED_HPP

#include "M5UnitComponent.hpp"
#include <M5HAL.hpp>
#include <vector>
#include <string>

class TwoWire;

/*!
  @namespace m5
  @brief Top level namespace of M5stack
 */
namespace m5 {
/*!
  @namespace unit
  @brief Unit-related namespace
 */
namespace unit {
class Component;

/*!
  @class UnitUnified
  @brief For managing and leading units
 */
class UnitUnified {
public:
    using container_type = std::vector<Component*>;

    ///@warning COPY PROHIBITED
    ///@name Constructor
    ///@{
    UnitUnified()                       = default;
    UnitUnified(const UnitUnified&)     = delete;
    UnitUnified(UnitUnified&&) noexcept = default;
    ///@}

    ///@warning COPY PROHIBITED
    ///@name Assignment
    ///@{
    UnitUnified& operator=(const UnitUnified&) = delete;

    UnitUnified& operator=(UnitUnified&&) noexcept = default;
    ///@}

    ///@name Adding unit to be managed
    ///@{
    // bool add(Component& u, Port& port);
    bool add(Component& u, m5::hal::bus::Bus* bus);
    //    [[deprecated("use add(Component& u, Port& port) or add(Component& u,
    //    m5::hal::bus::Bus* bus);")]]
    bool add(Component& u, TwoWire& wire);
    ///@}

    /*!
      @brief Begin of all units under management
      @param resetAndPlay Use the Reset and play function if true
      @return True if successful
      @note Reset and play is only available for supported units
    */
    bool begin(const bool resetAndPlay = false);
    //! @brief Update of all units under management
    void update(const bool force = false);

    ///@name Reset and play
    ///@{
    /*!
      @brief Retrieve the specified unit that has been successfully initiated
      @tparam T Unit class type
      @param prev Obtains the corresponding unit located next to the specified unit,
      Get first match if nullptr
      @retval pointer Unit
      @retval nullptr Not exists
     */
    template <class T>
    T* get(const T* prev = nullptr)
    {
        static_assert(std::is_base_of<Component, T>::value, "T muse be derived of Component");
        auto it = _units.begin();
        if (prev) {
            it = std::find(it, _units.end(), prev);
            if (it == _units.end()) {
                return nullptr;
            }
            ++it;
        }
        for (; it != _units.end(); ++it) {
            Component* u = *it;
            if (u->_begun && T::uid == u->identifier() && (T::attr & types::ATTRIBUTE_RESET_AND_PLAY)) {
                return static_cast<T*>(u);
            }
        }
        return nullptr;
    }

    //! @brief Output information for debug
    std::string debugInfo() const;

protected:
    bool add_children(Component& u);
    bool add(Component& u, m5::unit::Adapter* a);

    std::string make_unit_info(const Component* u, const uint8_t indent = 0) const;

protected:
    container_type _units{};

private:
    static uint32_t _registerCount;
};

}  // namespace unit
}  // namespace m5

#endif
