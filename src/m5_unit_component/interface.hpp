
#if 0
#include "adapter_spi.hpp"

namespace m5 {
namespace unit {

struct Interface
{

   std::shared_ptr<m5::unit::Adapter> _adapter{};
    

};



}}



readRegister8(0x01, val);
-> I2C
readRegister8I2C()
        adapter->wrie(), read()
-> SPI
readRegister8SPI()
        adapter->wrie(), read()
        

        Component(const uint8_t arg);// I2C なら reg SPIなら CS?

I2CComponent(reg);
SPICOmponent(cs_pin);

#endif
