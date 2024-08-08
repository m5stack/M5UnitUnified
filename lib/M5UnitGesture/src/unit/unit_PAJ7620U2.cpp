/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_PAJ7620U2.cpp
  @brief PAJ7620U2Unit for M5UnitUnified
*/
#include "unit_PAJ7620U2.hpp"
#include <M5Utility.hpp>
#include <array>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::paj7620u2;
using namespace m5::unit::paj7620u2::command;

namespace {
constexpr uint16_t chip_id{0x7620};
constexpr uint8_t wakeup_value{0x20};

#if 0
// Hz to IDLE_TIME
int freq_to_idle(const float frequency) {
    return static_cast<int>((1000.0f / frequency - 3.55f) / 0.0323f);
}
// IDLE_TIME tp Hz
float idle_to_freq(const int idleTime) {
    return 1000.0f / (idleTime * 0.0323f + 3.55f);
}
#endif

struct Pair {
    uint8_t reg, val;
};

// initialize parameter
constexpr Pair register_for_initialize[] = {
#if 1
    {0xEF, 0x00},  // Bank 0
    {0x41, 0x00},  //
    {0x42, 0x00},  //
    {0x37, 0x07},
    {0x38, 0x17},
    {0x39, 0x06},
    {0x42, 0x01},
    {0x46, 0x2D},
    {0x47, 0x0F},
    {0x48, 0x3C},
    {0x49, 0x00},
    {0x4A, 0x1E},
    {0x4C, 0x22},
    {0x51, 0x10},
    {0x5E, 0x10},
    {0x60, 0x27},
    {0x80, 0x42},
    {0x81, 0x44},
    {0x82, 0x04},
    {0x8B, 0x01},
    {0x90, 0x06},
    {0x95, 0x0A},
    {0x96, 0x0C},
    {0x97, 0x05},
    {0x9A, 0x14},
    {0x9C, 0x3F},
    {0xA5, 0x19},
    {0xCC, 0x19},
    {0xCD, 0x0B},
    {0xCE, 0x13},
    {0xCF, 0x64},
    {0xD0, 0x21},
    {0xEF, 0x01},  // Bank 1
    {0x02, 0x0F},
    {0x03, 0x10},
    {0x04, 0x02},
    {0x25, 0x01},
    {0x27, 0x39},
    {0x28, 0x7F},
    {0x29, 0x08},
    {0x3E, 0xFF},
    {0x5E, 0x3D},
    {0x65, 0x96},  // R_IDLE_TIME LSB - Set sensor speed to 'normal speed' - 120
                   // fps
    {0x67, 0x97},
    {0x69, 0xCD},
    {0x6A, 0x01},
    {0x6D, 0x2C},
    {0x6E, 0x01},
    {0x72, 0x01},
    {0x73, 0x35},
    {0x74, 0x00},  // Set to gesture mode
    {0x77, 0x01},
    {0xEF, 0x00},  // Bank 0
    {0x41, 0xFF},  // Re-enable interrupts for first 8 gestures
    {0x42, 0x01}   // Re-enable interrupts for wave gesture
#else
    {0xEF, 0x00},  // Bank 0
    {0x37, 0x07},  // R_CursorClampLeft [4:0]
    {0x38, 0x17},  // R_CursorClampRight [4:0]
    {0x39, 0x06},  // R_CursorClampUp [4:0]
    {0x42, 0x01},  // R_Int_2_En [7:0]
    {0x46, 0x2D},  // R_AELedOff_UB [7:0]
    {0x47, 0x0F},  // R_AELedOff_LB [7:0]
    {0x48, 0x3C},  // R_AE_Exposure_UB [7:0]
    {0x49, 0x00},  // R_AE_Exposure_UB [15:8]
    {0x4A, 0x1E},  // R_AE_Exposure_LB [7:0]
    {0x4C, 0x20},  // AE_Gain_UB [7:0]
    {0x51, 0x10},  // R_Manual_GG[0]
                   // Manual_Exposure[1],Manual_Exposure_Default[2],AE_EnH[4]
    {0x5E, 0x10},  // TG___CLK_manual[0],DMSP_CLK_manual[1],SEN__CLK_manual[2],
                   // THERCLK_manual[3],SRAM_CLK_manual[4],I2C__CLK_manual[5]
    {0x60, 0x27},  // TS_osc_code[6:0],OSC_BIST_OK[7]
    {0x80, 0x42},  // Im_GPIO0
    {0x81, 0x44},  // Tm_GPIO2_OEL
    {0x82, 0x04},  // Im_INT
    {0x8B, 0x01},  // R_Cursor_ObjectSizeTh [7:0]
    {0x90, 0x06},  // R_NoMotionCountThd [6:0]
    {0x95, 0x0A},  // R_ZDirectionThd [4:0]
    {0x96, 0x0C},  // R_ZDirectionXYThd [4:0]
    {0x97, 0x05},  // R_ZDirectionAngleThd [3:0]
    {0x9A, 0x14},  // R_RotateXYThd [4:0]
    {0x9C, 0x3F},  // R_FilterWeight [1:0], FilterDistThd [6:2]
    {0xA5, 0x19},  // R_FilterImage [0], R_FilterAverage_Mode [3:2],
                   // R_UseLightWeight [4]
    {0xCC, 0x19},  // R_YtoZSum[5:0]
    {0xCD, 0x0B},  // R_YtoZFactor[5:0]
    {0xCE, 0x13},  // R_PositionFilterLength[2:0],R_ProcessFilterLength[6:4]
    {0xCF, 0x64},  // R_WaveCountThd[3:0],R_WaveAngleThd[7:4]
    {0xD0, 0x21},  // R_AbortCountThd[2:0],R_AbortXYRatio[7:3]

    {0xEF, 0x01},  // Bank 1
    {0x02, 0x0F},  // Cmd_HStart [5:0]
    {0x03, 0x10},  // Cmd_VStart [5:0]
    {0x04, 0x02},  // R_HR_LS_Comp_DAvg_V Flip [0], VFlip [1], DAvg_H [2],
                   // DAvg_V [3], ASkip_H [4], ASkip_V [5], LS_Comp_DAvg_H [6],
                   // LS_Comp_DAvg_V[7]
    {0x25, 0x01},  // R_LensShadingComp_EnH [0]
    {0x27, 0x39},  // R_OffsetY [6:0]
    {0x28, 0x7F},  // R_LSC [6:0]
    {0x29, 0x08},  // R_LSFT [3:0]
    {0x3E, 0xFF},  // R_DebugPattern[7:0]
    {0x5E, 0x3D},  // T_clamp_drv_ctrl vbgp2vdday_byp_EnH [0],
                   // vdday_lvl [3:1], vbgp2vdda_byp_EnH [4],
                   //  vdda28comp_enh[5], clamp_drv_ctl [7:6]
    //    {0x65, 0x96},  // R_IDLE_TIME[7:0]
    // (1000/(IDLE_TIME*0.0323+3.55)、0x0096:120Hz、0x00A3:113Hz)
    {0x67, 0x97},  // R_IDLE_TIME_SLEEP_1 [7:0]
    {0x69, 0xCD},  // R_IDLE_TIME_SLEEP_2 [7:0]
    {0x6A, 0x01},  // R_IDLE_TIME_SLEEP_2 [15:8]
    {0x6D, 0x2C},  // R_Obj_TIME_2 [7:0]
    {0x6E, 0x01},  // R_Obj_TIME_2 [15:8]
    {0x72, 0x01},  // R_TG_EnH Enable/Disable PAJ7620U2[0]
    {0x73, 0x35},  // R_AUTO_SLEEP_Mode
    {0x77, 0x01},  // R_SRAM_Read_EnH[0]
#endif
};

// gesture mode
constexpr Pair register_for_gesture[] = {
#if 1
    {0xEF, 0x00},  // Bank 0
    {0x41, 0x00},  // Disable interrupts for first 8 gestures
    {0x42, 0x00},  // Disable wave (and other mode's) interrupt(s)
    {0x48, 0x3C},
    {0x49, 0x00},
    {0x51, 0x10},
    {0x83, 0x20},
    {0x9f, 0xf9},
    {0xEF, 0x01},  // Bank 1
    {0x01, 0x1E},
    {0x02, 0x0F},
    {0x03, 0x10},
    {0x04, 0x02},
    {0x41, 0x40},
    {0x43, 0x30},
    {0x65, 0x96},  // R_IDLE_TIME  - Normal mode LSB "120 fps" (supposedly)
    {0x66, 0x00},
    {0x67, 0x97},
    {0x68, 0x01},
    {0x69, 0xCD},
    {0x6A, 0x01},
    {0x6b, 0xb0},
    {0x6c, 0x04},
    {0x6D, 0x2C},
    {0x6E, 0x01},
    {0x74, 0x00},  // Set gesture mode
    {0xEF, 0x00},  // Bank 0
    {0x41, 0xFF},  // Re-enable interrupts for first 8 gestures
    {0x42, 0x01},  // Re-e
#else
    {0xEF, 0x00},  // Bank 0
    {0x41, 0x00},  // R_Int_1_En [7:0]
    {0x42, 0x00},  // R_Int_2_En [7:0]
    //{0xEF, 0x00},
    {0x48, 0x3C},  // R_AE_Exposure_UB [7:0]
    {0x49, 0x00},  // R_AE_Exposure_UB [15:8]
    {0x51, 0x10},  // R_Manual_GG[0]
                   // Manual_Exposure[1],Manual_Exposure_Default[2],AE_EnH[4]
    {0x83, 0x20},  // R_LightThd [7:0]
    {0x9f, 0xF9},  // R_RotateEnH

    {0xEF, 0x01},  // Bank 1
    {0x01, 0x1E},  // Cmd_VSize [5:0]
    {0x02, 0x0F},  // Cmd_HStart [5:0]
    {0x03, 0x10},  // Cmd_VStart [5:0]
    {0x04, 0x02},  // R_HR_LS_Comp_DAvg_V
    {0x41, 0x40},  // R_Int_1_En [7:0]
    {0x43, 0x30},  // IntFlag_1
    //    {0x65, 0x96},  // R_IDLE_TIME[7:0]
    {0x66, 0x00},  // R_IDLE_TIME[18:5]
    {0x67, 0x97},  // R_IDLE_TIME_SLEEP_1 [7:0]
    {0x68, 0x01},  // R_IDLE_TIME_SLEEP_1 [15:8]
    {0x69, 0xCD},  // R_IDLE_TIME_SLEEP_2 [7:0]
    {0x6A, 0x01},  // R_IDLE_TIME_SLEEP_2 [15:8]
    {0x6B, 0xb0},  // R_Obj_TIME_1 [7:0]
    {0x6C, 0x04},  // R_Obj_TIME_1 [15:8]
    {0x6D, 0x2C},  // R_Obj_TIME_2 [7:0]
    {0x6E, 0x01},  // R_Obj_TIME_2 [15:8]
    {0x74, 0x00},  // R_WakeUpSig_Sel 0:gesture
                   // Control_Mode[2:0],WakeUpSig_Sel[5:4],GPIO_Probe_En[7:6]
    {0xEF, 0x00},  // Bank 0
    {0x41, 0xFF},  // R_Int_1_En [7:0]
    {0x42, 0x01},  // R_Int_2_En [7:0]
#endif
    {0xFF /*terminator*/, 0xFF}};
// proximity mode
constexpr Pair register_for_proximity[] = {
    {0xEF, 0x00},  // Bank 0
    {0x41, 0x00},  // R_Int_1_En [7:0]
    {0x42, 0x02},  // R_Int_2_En [7:0]
    {0x48, 0x20},  // R_AE_Exposure_UB [7:0]
    {0x49, 0x00},  // R_AE_Exposure_UB [15:8]
    {0x51, 0x13},  // R_Manual_GG[0]
    {0x83, 0x00},  // R_LightThd [7:0]
    {0x9F, 0xF8},  // R_RotateEnH
    {0x69, 0x96},  // R_Pox_UB [7:0]
    {0x6A, 0x02},  // R_Pox_LB [7:0]
    {0xEF, 0x01},  // Bank 1
    {0x01, 0x1E},  // Cmd_VSize [5:0]
    {0x02, 0x0F},  // Cmd_HStart [5:0]
    {0x03, 0x10},  // Cmd_VStart [5:0]
    {0x04, 0x02},  // R_HR_LS_Comp_DAvg_V
    {0x41, 0x50},  // R_dac_ctrl
    {0x43, 0x34},  // R_pga_test
    {0x65, 0xCE},  // R_IDLE_TIME[7:0]
    {0x66, 0x0B},  // R_IDLE_TIME[18:5]
    {0x67, 0xCE},  // R_IDLE_TIME_SLEEP_1 [7:0]
    {0x68, 0x0B},  // R_IDLE_TIME_SLEEP_1 [15:8]
    {0x69, 0xE9},  // R_IDLE_TIME_SLEEP_2 [7:0]
    {0x6A, 0x05},  // R_IDLE_TIME_SLEEP_2 [15:8]
    {0x6B, 0x50},  // R_Obj_TIME_1 [7:0]
    {0x6C, 0xC3},  // R_Obj_TIME_1 [15:8]
    {0x6D, 0x50},  // R_Obj_TIME_2 [7:0]
    {0x6E, 0xC3},  // R_Obj_TIME_2 [15:8]
    {0x74, 0x05},  // R_WakeUpSig_SelEnable 5:proximity
    {0xFF /*terminator*/, 0xFF}};
// cursor mode
constexpr Pair register_for_cursor[] = {
    // restore
    {0Xef, 0X00},
    {0X48, 0X3C},
    {0X49, 0X00},
    {0X51, 0X10},
    {0X83, 0X20},
    {0X9f, 0XF9},
    {0X69, 0X14},
    {0X6a, 0X0A},
    {0xEF, 0x00},  // Set Bank 0
    {0x32, 0x29},  // R_CursorClampLeft
    {0x33, 0x01},  // R_PositionFilterStartSizeTh [7:0]
    {0x34, 0x00},  // R_PositionFilterStartSizeTh [8]
    {0x35, 0x01},  // R_ProcessFilterStartSizeTh [7:0]
    {0x36, 0x00},  // R_ProcessFilterStartSizeTh [8]
    {0x37, 0x03},  // R_CursorClampLeft [4:0]
    {0x38, 0x1B},  // R_CursorClampRight [4:0]
    {0x39, 0x03},  // R_CursorClampUp [4:0]
    {0x3A, 0x1B},  // R_CursorClampDown [4:0]
    {0x41, 0x00},  // R_Int_1_En [7:0]
    {0x42, 0x84},  // R_Int_2_En [7:0]
    {0x8B, 0x01},  // R_Cursor_ObjectSizeTh [7:0]
    {0x8C, 0x07},  // R_PositionResolution [2:0]
    {0xEF, 0x01},  // Bank 1
    {0x04, 0x03},  // R_HR_LS_Comp_DAvg_V (XY flip)
    {0x74, 0x03},  // R_WakeUpSig_SelEnable 3:cursor
    {0xFF /*terminator*/, 0xFF}};

constexpr const Pair* register_table[] = {
    register_for_gesture,
    register_for_proximity,
    register_for_cursor,
};

constexpr Gesture rotate_1[] = {
    Gesture::Left,
    Gesture::Right,
    Gesture::Down,
    Gesture::Up,

};
constexpr Gesture rotate_2[] = {
    Gesture::Down,
    Gesture::Up,
    Gesture::Right,
    Gesture::Left,
};
constexpr Gesture rotate_3[] = {
    Gesture::Right,
    Gesture::Left,
    Gesture::Up,
    Gesture::Down,
};
// CCW
constexpr const Gesture* rotate_table[] = {
    nullptr,
    rotate_1,
    rotate_2,
    rotate_3,
};

Gesture rotate_gesture(const Gesture g, const uint8_t rot) {
    auto gv = m5::stl::to_underlying(g);
    auto p  = rotate_table[rot & 0x03];
    if (!p || (gv & 0x0F) == 0) {
        return g;
    }
    return static_cast<Gesture>(p[__builtin_ctz(gv)]);
}

constexpr uint8_t freq_table[] = {
    0xAC,  // 120
    0x30,  // 240
};

}  // namespace

namespace m5 {
namespace unit {
// class UnitPAJ7620U2
const char UnitPAJ7620U2::name[] = "UnitPAJ7620U2";
const types::uid_t UnitPAJ7620U2::uid{"UnitPAJ7620U2"_mmh3};
const types::uid_t UnitPAJ7620U2::attr{0};

bool UnitPAJ7620U2::begin() {
    assert(_cfg.stored_size && "stored_size must be greater than zero");
    if (_cfg.stored_size != _data->capacity()) {
        _data.reset(new m5::container::CircularBuffer<Data>(_cfg.stored_size));
        if (!_data) {
            M5_LIB_LOGE("Failed to allocate");
            return false;
        }
    }

    uint16_t id{};
    uint8_t ver{};

    m5::utility::delay(1);  // Wait 700us for PAJ7620U2 to stabilize

    // call select_bank twice for wakeup
    select_bank(0, true);
    select_bank(0, true);
    if (!was_wakeup()) {
        M5_LIB_LOGE("Failed to wait wakeup");
        return false;
    }

    // Check chip ID and get version
    if (!get_chip_id(id) || !get_version(ver)) {
        M5_LIB_LOGE("Failed to get id/version %x:%x", id, ver);
        return false;
    }
    if (id != chip_id) {
        M5_LIB_LOGE("Not PAJ7620U2 %x", id);
        return false;
    }

    // rotation
    _rotation = _cfg.rotation & 0x03;

    // Set initialize value to registers
    for (auto&& e : register_for_initialize) {
        if (!writeRegister8(e.reg, e.val)) {
            M5_LIB_LOGE("Failed to initilize [%02x]:%x", e.reg, e.val);
            return false;
        }
    }
    if (!select_bank(0, true) || !setFrequency(_cfg.frequency) ||
        !setMode(_cfg.mode)) {
        M5_LIB_LOGE("Fauled to settings");
        return false;
    }

    return _cfg.start_periodic ? startPeriodicMeasurement() : true;
}

void UnitPAJ7620U2::update(const bool force) {
    _updated = false;
    if (inPeriodic()) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            Data d{};
            switch (_mode) {
                case Mode::Gesture:
                    _updated = update_gesture(d);
                    if (_updated && _cfg.store_on_change && !empty()) {
                        _updated = latest().gesture() != d.gesture();
                    }
                    break;
                case Mode::Proximity:
                    _updated = update_proximity(d);
                    if (_updated && _cfg.store_on_change && !empty()) {
                        _updated = latest().gesture() != d.gesture() ||
                                   latest().brightness() != d.brightness() ||
                                   latest().approach() != d.approach();
                    }
                    break;
                case Mode::Cursor:
                    _updated = update_cursor(d);
                    if (_updated && _cfg.store_on_change && !empty()) {
                        _updated = latest().gesture() != d.gesture() ||
                                   latest().cursorX() != d.cursorX() ||
                                   latest().cursorY() != d.cursorY();
                    }
                    break;
                default:
                    return;
            }
            if (_updated) {
                _latest     = at;
                d.data_mode = _mode;
                _data->push_back(d);
            }
        }
    }
}

bool UnitPAJ7620U2::update_gesture(paj7620u2::Data& d) {
#if 0
    Gesture ges{};
    if (readGesture(ges) && ges != _gesture) {
        _gesture = ges;
        return true;
    }
    return false;
#else
    if (read_gesture(d)) {
        d.data_gesture = rotate_gesture(
            static_cast<Gesture>(*(uint16_t*)d.raw.data()), _rotation);
        return true;
    }
    return false;
#endif
}

bool UnitPAJ7620U2::update_proximity(paj7620u2::Data& d) {
#if 0
    uint8_t br{}, app{};
    Gesture ges{};
    if (readGesture(ges) && readProximity(br, app)) {
        if (ges != _gesture || (br != _brightness || app != _approach)) {
            _gesture    = ges;
            _brightness = br;
            _approach   = (bool)app;
            return true;
        }
    }
    return false;
#else
    if (read_gesture(d) && read_proximity(d)) {
        d.proximity_brightness = d.raw[2];
        d.proximity_approach   = d.raw[3];
        return true;
    }
    return false;
#endif
}

bool UnitPAJ7620U2::update_cursor(paj7620u2::Data& d) {
#if 0
    Gesture ges{};
    uint16_t x{_cursorX}, y{_cursorY};

    if (readGesture(ges)) {
        if (ges == Gesture::HasObject) {
            if (!readCursor(x, y)) {
                return false;
            }
        }
        if (ges != _gesture || (x != _cursorX || y != _cursorY)) {
            _gesture = ges;
            _cursorX = x;
            _cursorY = y;
            return true;
        }
    }
    return false;
#else
    if (read_gesture(d) && d.gesture() == Gesture::HasObject &&
        read_cursor(d)) {
        d.cursor_x = (((uint16_t)(d.raw[3] & 0X0F)) << 8) | d.raw[2];
        d.cursor_y = (((uint16_t)(d.raw[5] & 0X0F)) << 8) | d.raw[4];
        return true;
    }
    return false;

#endif
}

bool UnitPAJ7620U2::read_gesture(Data& d) {
    return read_banked_register(INT_FLAG_1, d.raw.data(), d.raw.size());
}

bool UnitPAJ7620U2::read_proximity(Data& d) {
    return read_banked_register(S_AVGY, d.raw.data() + 2, 1) &&
           read_banked_register(S_STATE, d.raw.data() + 3, 1);
}

bool UnitPAJ7620U2::read_cursor(Data& d) {
    return read_banked_register(OBJECT_CENTER_X_LOW, d.raw.data() + 2, 4);
}

bool UnitPAJ7620U2::readGesture(Gesture& ges) {
#if 1
    if (read_banked_register(INT_FLAG_1, (uint8_t*)&ges, 2)) {
        ges = rotate_gesture(ges, _rotation);
        return true;
    }
#else
    uint8_t high{}, low{};
    if (read_banked_register8(INT_FLAG_1, low) &&
        read_banked_register8(INT_FLAG_2, high)) {
        uint16_t v = (((uint16_t)high) << 8) | low;
        ges        = static_cast<Gesture>(v);
        ges        = rotate_gesture(ges, _rotation);
        return true;
    }
#endif
    return false;
}

bool UnitPAJ7620U2::readNoObjectCount(uint8_t& cnt) {
    return read_banked_register8(NO_OBJECT_COUNT, cnt);
}

bool UnitPAJ7620U2::readNoMotionCount(uint8_t& cnt) {
    return read_banked_register8(NO_MOTION_COUNT, cnt);
}

bool UnitPAJ7620U2::readObjectSize(uint16_t& sz) {
    return read_banked_register(OBJECT_SIZE_LOW, (uint8_t*)&sz, 2);
}

bool UnitPAJ7620U2::readProximity(uint8_t& brightness, uint8_t& approach) {
    return read_banked_register8(S_AVGY, brightness) &&
           read_banked_register8(S_STATE, approach);
}

bool UnitPAJ7620U2::readObjectCenter(uint16_t& x, uint16_t& y) {
    uint8_t xl{}, xh{}, yl{}, yh{};
    if (read_banked_register8(OBJECT_CENTER_X_LOW, xl) &&
        read_banked_register8(OBJECT_CENTER_X_HIGH, xh) &&
        read_banked_register8(OBJECT_CENTER_Y_LOW, yl) &&
        read_banked_register8(OBJECT_CENTER_Y_HIGH, yh)) {
        x = (((uint16_t)(xh & 0X1F)) << 8) | xl;
        y = (((uint16_t)(yh & 0X1F)) << 8) | yl;
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::readCursor(uint16_t& x, uint16_t& y) {
    std::array<uint8_t, 4> raw{};  // X_LOW , 4?

    uint8_t xl{}, xh{}, yl{}, yh{};
    if (read_banked_register8(CURSOR_CLAMP_CENTER_X_LOW, xl) &&
        read_banked_register8(CURSOR_CLAMP_CENTER_X_HIGH, xh) &&
        read_banked_register8(CURSOR_CLAMP_CENTER_Y_LOW, yl) &&
        read_banked_register8(CURSOR_CLAMP_CENTER_Y_HIGH, yh)) {
        x = (((uint16_t)(xh & 0X0F)) << 8) | xl;
        y = (((uint16_t)(yh & 0X0F)) << 8) | yl;
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::enable(const bool flag) {
    return write_banked_register8(R_TG_ENH, flag ? 1 : 0);
}

bool UnitPAJ7620U2::suspend() {
    return enable(false) && write_banked_register8(SW_SUSPEND_ENL, 0);
}

bool UnitPAJ7620U2::resume() {
    return was_wakeup() && enable(true);
}

bool UnitPAJ7620U2::readFrequency(uint8_t& raw) {
    return read_banked_register8(R_REF_CLK_CNT_LOW, raw);
}

bool UnitPAJ7620U2::readFrequency(Frequency& f) {
    f = Frequency::Unknown;

    uint8_t raw{};
    if (readFrequency(raw)) {
        int8_t idx{};
        for (auto&& e : freq_table) {
            if (e == raw) {
                f = static_cast<Frequency>(idx);
                return true;
            }
            ++idx;
        }
    }
    return false;
}

bool UnitPAJ7620U2::setFrequency(const Frequency f) {
    if (f == Frequency::Unknown ||
        !write_banked_register8(R_REF_CLK_CNT_LOW,
                                freq_table[m5::stl::to_underlying(f)])) {
        return false;
    }
    _frequency = f;
    return true;
}

bool UnitPAJ7620U2::setMode(const Mode m) {
    auto idx = m5::stl::to_underlying(m);
    const Pair* rv =
        idx < m5::stl::size(register_table) ? register_table[idx] : nullptr;
    if (!rv) {
        M5_LIB_LOGE("Inbalid mode:%x", m);
        return false;
    }

    while (rv->reg != 0xFF) {
#if 0
        uint8_t v{};
        if (!readRegister8(rv->reg, v, 0)) {
            return false;
        }
        M5_LIB_LOGE("{0X%02x,0X%02X}", rv->reg, v);
        // M5_LIB_LOGI("[%02X]:%02X", rv->reg, rv->val);
#endif
        if (!writeRegister8(rv->reg, rv->val)) {
            M5_LIB_LOGE("Failed to change mode %x:%x", rv->reg, rv->val);
            return false;
        }
        ++rv;
    }
    _mode = m;

    // To resolve bank inconsistencies after register setting
    return select_bank(0, true) &&
           ((_mode != Mode::Proximity) ? setFrequency(_frequency) : true);
}

bool UnitPAJ7620U2::readApproachThreshold(uint8_t& high, uint8_t& low) {
    return read_banked_register8(R_POX_UB, high) &&
           read_banked_register8(R_POX_LB, low);
}

bool UnitPAJ7620U2::setApproachThreshold(const uint8_t high,
                                         const uint8_t low) {
    return write_banked_register8(R_POX_UB, high) &&
           write_banked_register8(R_POX_LB, low);
}

bool UnitPAJ7620U2::readHorizontalFlip(bool& flip) {
    uint8_t v{};
    if (read_banked_register8(LS_COMP_DAVG_V, v)) {
        flip = (v & 0x01);  // HFlip bit:0
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::readVerticalFlip(bool& flip) {
    uint8_t v{};
    if (read_banked_register8(LS_COMP_DAVG_V, v)) {
        flip = (v & 0x02);  // VFlip bit:1
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::setHorizontalFlip(const bool flip) {
    uint8_t v{};
    if (read_banked_register8(LS_COMP_DAVG_V, v)) {
        v = (v & ~0x01) | (flip ? 0x01 : 0x00);
        return writeRegister8((uint8_t)(LS_COMP_DAVG_V & 0xFF), v);
    }
    return false;
}

bool UnitPAJ7620U2::setVerticalFlip(const bool flip) {
    uint8_t v{};
    if (read_banked_register8(LS_COMP_DAVG_V, v)) {
        v = (v & ~0x02) | (flip ? 0x02 : 0x00);
        return writeRegister8((uint8_t)(LS_COMP_DAVG_V & 0xFF), v);
    }
    return false;
}

//
bool UnitPAJ7620U2::select_bank(const uint8_t bank, const bool force) {
    if (!force && _current_bank == bank) {
        return true;
    }
    if (writeRegister8(BANK_SEL, bank)) {
        _current_bank = bank;
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::read_banked_register(const uint16_t reg, uint8_t* buf,
                                         const size_t len) {
    return select_bank((reg >> 8) & 1) &&
           readRegister((uint8_t)(reg & 0xFF), buf, len, 1);
}

bool UnitPAJ7620U2::read_banked_register8(const uint16_t reg, uint8_t& value) {
    return select_bank((reg >> 8) & 1) &&
           readRegister8((uint8_t)(reg & 0xFF), value, 1);
}

bool UnitPAJ7620U2::read_banked_register16(const uint16_t reg,
                                           uint16_t& value) {
    return select_bank((reg >> 8) & 1) &&
           readRegister16((uint8_t)(reg & 0xFF), value, 1);
}

bool UnitPAJ7620U2::write_banked_register(const uint16_t reg,
                                          const uint8_t* buf,
                                          const size_t len) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister((uint8_t)(reg & 0xFF), buf, len);
}

bool UnitPAJ7620U2::write_banked_register8(const uint16_t reg,
                                           const uint8_t value) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister8((uint8_t)(reg & 0xFF), value);
}

bool UnitPAJ7620U2::write_banked_register16(const uint16_t reg,
                                            const uint16_t value) {
    return select_bank((reg >> 8) & 1) &&
           writeRegister16((uint8_t)(reg & 0xFF), value);
}

bool UnitPAJ7620U2::was_wakeup() {
    uint8_t v{};
    return read_banked_register8(PART_ID_LOW, v) && (v == wakeup_value);
}

bool UnitPAJ7620U2::get_chip_id(uint16_t& id) {
    return read_banked_register(PART_ID_LOW, (uint8_t*)&id, 2);
}

bool UnitPAJ7620U2::get_version(uint8_t& version) {
    return read_banked_register8(VERSION_ID, version);
}

bool UnitPAJ7620U2::start_periodic_measurement(const paj7620u2::Mode mode,
                                               const paj7620u2::Frequency freq,
                                               const uint32_t intervalMs) {
    if (inPeriodic()) {
        return false;
    }
    return setFrequency(freq) && setMode(mode) &&
           start_periodic_measurement(intervalMs);
}

bool UnitPAJ7620U2::start_periodic_measurement(const uint32_t intervalMs) {
    if (inPeriodic()) {
        return false;
    }
    _interval = intervalMs;
    _latest   = 0;
    _periodic = true;
    return true;
}

bool UnitPAJ7620U2::stop_periodic_measurement() {
    _periodic = false;
    return true;
}

}  // namespace unit
}  // namespace m5

