/*!
  @file M5UnitUnified.hpp
  @brief Main header of M5UnitUnified

  @mainpage M5UnitUnfied
  Library for abstracted M5 units.
  C++11 or later.
  
  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UNIT_UNIFIED_HPP
#define M5_UNIT_UNIFIED_HPP

#include <M5UnitComponent.hpp>
#include <M5HAL.hpp>
#include <vector>
#include <string>

class TwoWire;
namespace m5 {
namespace unit {
class Component;

/*!
  @class UnitUnified
  @brief For managing and leading units
 */
class UnitUnified {
   public:
    using container_type = std::vector<Component*>;

    ///@name Constructor
    ///@{
    UnitUnified()                              = default;
    UnitUnified(const UnitUnified&)            = delete;
    UnitUnified(UnitUnified&&)                 = default;
    ///@}

    ///@name Assignment
    ///@{
    UnitUnified& operator=(const UnitUnified&) = delete;
    UnitUnified& operator=(UnitUnified&&)      = default;
    ///@}

    ///@name Adding unit to be managed
    ///@{
    //bool add(Component& u, Port& port);
    bool add(Component& u, m5::hal::bus::Bus* bus);
    //    [[deprecated("use add(Component& u, Port& port) or add(Component& u, m5::hal::bus::Bus* bus);")]]
    bool add(Component& u, TwoWire& wire);
    ///@}
    
    bool begin();
    void update();

    //! @brief Output information for debug
    std::string debugInfo() const;
    
  protected:
    bool add_children(Component& u);
    bool add(Component& u, m5::unit::Adapter* a);

    std::string make_unit_info(const Component* u, const uint8_t indent = 0) const;
    
   protected:
    container_type _units;

   private:
    static uint32_t _registerCount;
};

}  // namespace unit
}  // namespace m5

#endif
