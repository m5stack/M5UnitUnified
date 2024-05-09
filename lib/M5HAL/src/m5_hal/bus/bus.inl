
#include "bus.hpp"

namespace m5 {
namespace hal {
namespace bus {

const BusConfig& Accessor::getBusConfig(void) const
{
    return _bus.getConfig();
}

} // namespace bus
} // namespace hal
} // namespace m5
