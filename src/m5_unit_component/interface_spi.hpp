/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file interface_spi.hpp
  @brief Interfacw for SPI access
*/
#if 0

#ifndef M5_UNIT_COMPONENT_INTERFACE_SPI_HPP
#define M5_UNIT_COMPONENT_INTERFACE_SPI_HPP

#include <ctsdint>
#include "types.hpp"
#include "adapter_spi.hpp"

namespace m5 {
namespace unit {

struct InterfaceSPI {
    bool readRegisterSPI(const uint8_t reg, uint8_t* data, const uint32_t len);
    bool writeRegisterSPI(const uint8_t reg, uint8_t* data, const uint32_t len);
    //    std::shared_ptr<m5::unit::SPIAdapter> _spi_adapter{};
};

}  // namespace unit
}  // namespace m5



struct InterfaceI2C {
    bool readRegisterI2C(const uint8_t reg, uint8_t* data, const uint32_t len);
    bool writeRegisterI2C(const uint8_t reg, uint8_t* data, const uint32_t len);
    //    std::shared_ptr<m5::unit::I2CAdapter> _i2c_adapter{};
};




void read_reg8()
{
    return interface()->read(a,b);


}
#endif

