/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Host (native) build check for M5UnitUnified

  Not a test: it only has to compile, link and exit 0. It pulls in every public
  header and instantiates the templates defined in headers, so that a missing
  ESP_PLATFORM guard or a compiler extension surfaces on GCC / Clang / MSVC.
  The functions below are compiled but never called (there is no bus on the host).
*/
#include <cstdint>
#include <cstddef>

#include <M5UnitUnified.hpp>
#include <wiring/m5_unit_unified_wiring.hpp>

namespace m5 {
namespace unit {
namespace native_check {

struct Data {
    uint16_t value{};
};

// Minimal unit exercising the Component templates and PeriodicMeasurementAdapter
class UnitCheck : public Component, public PeriodicMeasurementAdapter<UnitCheck, Data> {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitCheck, 0x42);

public:
    UnitCheck() : Component(DEFAULT_ADDRESS)
    {
    }

    template <typename Reg>
    bool exerciseRegisters(const Reg reg)
    {
        uint8_t buf[4]{};
        uint8_t v8{};
        uint16_t v16{};
        uint32_t v32{};
        bool ok = readRegister(reg, buf, sizeof(buf), 0);
        ok &= readRegister8(reg, v8, 0);
        ok &= readRegister16BE(reg, v16, 0);
        ok &= readRegister16LE(reg, v16, 0);
        ok &= readRegister32BE(reg, v32, 0);
        ok &= readRegister32LE(reg, v32, 0);
        ok &= writeRegister(reg, buf, sizeof(buf));
        ok &= writeRegister8(reg, v8);
        ok &= writeRegister16BE(reg, v16);
        ok &= writeRegister16LE(reg, v16);
        ok &= writeRegister32BE(reg, v32);
        ok &= writeRegister32LE(reg, v32);
        return ok;
    }

    bool exerciseAdapter() const
    {
        return asAdapter<AdapterI2C>(Adapter::Type::I2C) != nullptr;
    }

protected:
    friend class PeriodicMeasurementAdapter<UnitCheck, Data>;

    bool start_periodic_measurement()
    {
        return true;
    }
    bool stop_periodic_measurement()
    {
        return true;
    }
    Data oldest_periodic_data() const
    {
        return _data;
    }
    Data latest_periodic_data() const
    {
        return _data;
    }
    size_t available_periodic_measurement_data() const override
    {
        return 0;
    }
    bool empty_periodic_measurement_data() const override
    {
        return true;
    }
    bool full_periodic_measurement_data() const override
    {
        return false;
    }
    void discard_periodic_measurement_data() override
    {
    }
    void flush_periodic_measurement_data() override
    {
    }

private:
    Data _data{};
};

const char UnitCheck::name[] = "UnitCheck";
const types::uid_t UnitCheck::uid{0x4E415456u};  // "NATV"
const types::attr_t UnitCheck::attr{types::attribute::AccessI2C};

}  // namespace native_check
}  // namespace unit
}  // namespace m5

// External linkage keeps these from being discarded before instantiation
bool native_check_exercise(m5::unit::native_check::UnitCheck& u)
{
    bool ok = u.exerciseRegisters<uint8_t>(0x00);
    ok &= u.exerciseRegisters<uint16_t>(0x0000);
    ok &= u.exerciseAdapter();
    ok &= u.startPeriodicMeasurement();
    ok &= u.stopPeriodicMeasurement();
    ok &= !u.empty() || u.full() || u.available() > 0;
    (void)u.oldest();
    (void)u.latest();
    u.discard();
    u.flush();
    return ok;
}

int main()
{
    m5::unit::UnitUnified units;
    m5::unit::native_check::UnitCheck unit;
    (void)units;
    (void)unit;
    return 0;
}
