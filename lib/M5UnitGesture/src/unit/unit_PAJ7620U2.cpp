/*!
  @file unit_PAJ7620U2.cpp
  @brief PAJ7620U2Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_PAJ7620U2.hpp"
#include <M5Utility.hpp>
#include <array>

using namespace m5::utility::mmh3;
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
    {0xEF, 0x00},  // bank 0
    {0x32, 0x29}, {0x33, 0x01}, {0x34, 0x00}, {0x35, 0x01}, {0x36, 0x00},
    {0x37, 0x07}, {0x38, 0x17}, {0x39, 0x06}, {0x3A, 0x12}, {0x3F, 0x00},
    {0x40, 0x02}, {0x41, 0xFF}, {0x42, 0x01}, {0x46, 0x2D}, {0x47, 0x0F},
    {0x48, 0x3C}, {0x49, 0x00}, {0x4A, 0x1E}, {0x4B, 0x00}, {0x4C, 0x20},
    {0x4D, 0x00}, {0x4E, 0x1A}, {0x4F, 0x14}, {0x50, 0x00}, {0x51, 0x10},
    {0x52, 0x00}, {0x5C, 0x02}, {0x5D, 0x00}, {0x5E, 0x10}, {0x5F, 0x3F},
    {0x60, 0x27}, {0x61, 0x28}, {0x62, 0x00}, {0x63, 0x03}, {0x64, 0xF7},
    {0x65, 0x03}, {0x66, 0xD9}, {0x67, 0x03}, {0x68, 0x01}, {0x69, 0xC8},
    {0x6A, 0x40}, {0x6D, 0x04}, {0x6E, 0x00}, {0x6F, 0x00}, {0x70, 0x80},
    {0x71, 0x00}, {0x72, 0x00}, {0x73, 0x00}, {0x74, 0xF0}, {0x75, 0x00},
    {0x80, 0x42}, {0x81, 0x44}, {0x82, 0x04}, {0x83, 0x20}, {0x84, 0x20},
    {0x85, 0x00}, {0x86, 0x10}, {0x87, 0x00}, {0x88, 0x05}, {0x89, 0x18},
    {0x8A, 0x10}, {0x8B, 0x01}, {0x8C, 0x37}, {0x8D, 0x00}, {0x8E, 0xF0},
    {0x8F, 0x81}, {0x90, 0x06}, {0x91, 0x06}, {0x92, 0x1E}, {0x93, 0x0D},
    {0x94, 0x0A}, {0x95, 0x0A}, {0x96, 0x0C}, {0x97, 0x05}, {0x98, 0x0A},
    {0x99, 0x41}, {0x9A, 0x14}, {0x9B, 0x0A}, {0x9C, 0x3F}, {0x9D, 0x33},
    {0x9E, 0xAE}, {0x9F, 0xF9}, {0xA0, 0x48}, {0xA1, 0x13}, {0xA2, 0x10},
    {0xA3, 0x08}, {0xA4, 0x30}, {0xA5, 0x19}, {0xA6, 0x10}, {0xA7, 0x08},
    {0xA8, 0x24}, {0xA9, 0x04}, {0xAA, 0x1E}, {0xAB, 0x1E}, {0xCC, 0x19},
    {0xCD, 0x0B}, {0xCE, 0x13}, {0xCF, 0x64}, {0xD0, 0x21}, {0xD1, 0x0F},
    {0xD2, 0x88}, {0xE0, 0x01}, {0xE1, 0x04}, {0xE2, 0x41}, {0xE3, 0xD6},
    {0xE4, 0x00}, {0xE5, 0x0C}, {0xE6, 0x0A}, {0xE7, 0x00}, {0xE8, 0x00},
    {0xE9, 0x00}, {0xEE, 0x07}, {0xEF, 0x01},  // bank 1
    {0x00, 0x1E}, {0x01, 0x1E}, {0x02, 0x0F}, {0x03, 0x10}, {0x04, 0x02},
    {0x05, 0x00}, {0x06, 0xB0}, {0x07, 0x04}, {0x08, 0x0D}, {0x09, 0x0E},
    {0x0A, 0x9C}, {0x0B, 0x04}, {0x0C, 0x05}, {0x0D, 0x0F}, {0x0E, 0x02},
    {0x0F, 0x12}, {0x10, 0x02}, {0x11, 0x02}, {0x12, 0x00}, {0x13, 0x01},
    {0x14, 0x05}, {0x15, 0x07}, {0x16, 0x05}, {0x17, 0x07}, {0x18, 0x01},
    {0x19, 0x04}, {0x1A, 0x05}, {0x1B, 0x0C}, {0x1C, 0x2A}, {0x1D, 0x01},
    {0x1E, 0x00}, {0x21, 0x00}, {0x22, 0x00}, {0x23, 0x00}, {0x25, 0x01},
    {0x26, 0x00}, {0x27, 0x39}, {0x28, 0x7F}, {0x29, 0x08}, {0x30, 0x03},
    {0x31, 0x00}, {0x32, 0x1A}, {0x33, 0x1A}, {0x34, 0x07}, {0x35, 0x07},
    {0x36, 0x01}, {0x37, 0xFF}, {0x38, 0x36}, {0x39, 0x07}, {0x3A, 0x00},
    {0x3E, 0xFF}, {0x3F, 0x00}, {0x40, 0x77}, {0x41, 0x40}, {0x42, 0x00},
    {0x43, 0x30}, {0x44, 0xA0}, {0x45, 0x5C}, {0x46, 0x00}, {0x47, 0x00},
    {0x48, 0x58}, {0x4A, 0x1E}, {0x4B, 0x1E}, {0x4C, 0x00}, {0x4D, 0x00},
    {0x4E, 0xA0}, {0x4F, 0x80}, {0x50, 0x00}, {0x51, 0x00}, {0x52, 0x00},
    {0x53, 0x00}, {0x54, 0x00}, {0x57, 0x80}, {0x59, 0x10}, {0x5A, 0x08},
    {0x5B, 0x94}, {0x5C, 0xE8}, {0x5D, 0x08}, {0x5E, 0x3D}, {0x5F, 0x99},
    {0x60, 0x45}, {0x61, 0x40}, {0x63, 0x2D}, {0x64, 0x02}, {0x65, 0x96},
    {0x66, 0x00}, {0x67, 0x97}, {0x68, 0x01}, {0x69, 0xCD}, {0x6A, 0x01},
    {0x6B, 0xB0}, {0x6C, 0x04}, {0x6D, 0x2C}, {0x6E, 0x01}, {0x6F, 0x32},
    {0x71, 0x00}, {0x72, 0x01}, {0x73, 0x35}, {0x74, 0x00}, {0x75, 0x33},
    {0x76, 0x31}, {0x77, 0x01}, {0x7C, 0x84}, {0x7D, 0x03}, {0x7E, 0x01},
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
    return select_bank(0, true) && setMode(_cfg.mode) &&
           ((_cfg.detection != DetectionMode::Gesture)
                ? setDetectionMode(_cfg.detection)
                : true);
}

void UnitPAJ7620U2::update() {
    switch (_detection) {
        case DetectionMode::Gesture:
            _updated = update_gesture();
            break;
        case DetectionMode::Proximity:
            _updated = update_proximity();
            break;
        case DetectionMode::Cursor:
            _updated = update_cursor();
            break;
        default:
            _updated = false;
            break;
    }
    if (_updated) {
        _latest = m5::utility::millis();
    }
}

bool UnitPAJ7620U2::update_gesture() {
    Gesture ges{};
    if (readGesture(ges) && ges != _gesture) {
        _gesture = ges;
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::update_proximity() {
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
}

bool UnitPAJ7620U2::update_cursor() {
    Gesture ges{};
    uint16_t x{}, y{};
    if (readGesture(ges) && readCursor(x, y)) {
        if (ges != _gesture || (x != _cursorX || y != _cursorY)) {
            _cursorX = x;
            _cursorY = y;
            return true;
        }
    }
    return false;
}

bool UnitPAJ7620U2::readGesture(Gesture& ges) {
#if 1
    if (!read_banked_register(INT_FLAG_1, (uint8_t*)&ges, 2)) {
        return false;
    }
    ges = rotate_gesture(ges, _rotation);
    return true;
#else

#endif
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
    uint8_t xl{}, xh{}, yl{}, yh{};
    if (read_banked_register8(CURSOR_CLAMP_CEMTER_X_LOW, xl) &&
        read_banked_register8(CURSOR_CLAMP_CEMTER_X_HIGH, xh) &&
        read_banked_register8(CURSOR_CLAMP_CEMTER_Y_LOW, yl) &&
        read_banked_register8(CURSOR_CLAMP_CEMTER_Y_HIGH, yh)) {
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

bool UnitPAJ7620U2::setMode(const Mode m) {
    if (!write_banked_register8(R_REF_CLK_CNT_LOW,
                                freq_table[m5::stl::to_underlying(m)])) {
        return false;
    }
    _mode    = m;
    _gesture = Gesture::None;
    _cursorX = _cursorY = 0xFFFF;
    _brightness         = 0;
    _approach           = false;
    return true;
}

bool UnitPAJ7620U2::setDetectionMode(const DetectionMode m) {
    auto idx = m5::stl::to_underlying(m);
    const Pair* rv =
        idx < m5::stl::size(register_table) ? register_table[idx] : nullptr;
    if (!rv) {
        M5_LIB_LOGE("Inbalid mode:%x", m);
        return false;
    }
    while (rv->reg != 0xFF) {
        // M5_LIB_LOGI("[%02X]:%02X", rv->reg, rv->val);
        if (!writeRegister8(rv->reg, rv->val)) {
            M5_LIB_LOGE("Failed to change mode %x:%x", rv->reg, rv->val);
            return false;
        }
        ++rv;
    }
    _detection = m;

    // To resolve bank inconsistencies after register setting
    return select_bank(0, true) &&
           ((_detection != DetectionMode::Proximity) ? setMode(_mode) : true);
}

bool UnitPAJ7620U2::getApprochThreshold(uint8_t& high, uint8_t& low) {
    return read_banked_register8(R_POX_UB, high) &&
           read_banked_register8(R_POX_LB, low);
}

bool UnitPAJ7620U2::setApprochThreshold(const uint8_t high, const uint8_t low) {
    return write_banked_register8(R_POX_UB, high) &&
           write_banked_register8(R_POX_LB, low);
}

bool UnitPAJ7620U2::getHorizontalFlip(bool& flip) {
    uint8_t v{};
    if (read_banked_register8(LS_COMP_DAVG_V, v)) {
        flip = (v & 0x01);  // HFlip bit:0
        return true;
    }
    return false;
}

bool UnitPAJ7620U2::getVerticalFlip(bool& flip) {
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

}  // namespace unit
}  // namespace m5
