/*!
  @file unit_MFRC522.cpp
  @brief MFRC522 Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#include "unit_MFRC522.hpp"

#include <M5Utility.hpp>
#include <cassert>
#include <array>
#include <thread>
#include <cstdio>

namespace {
using namespace m5::unit::mfrc522;

// Firmware data for self-test
// Reference values based on firmware version

// Version 0.0 (0x90)
// Philips Semiconductors; Preliminary Specification Revision 2.0 - 01 August
// 2005; 16.1 Sefttest
constexpr std::array<uint8_t, 64> firmware_referenceV0_0{
    0x00, 0x87, 0x98, 0x0f, 0x49, 0xFF, 0x07, 0x19, 0xBF, 0x22, 0x30,
    0x49, 0x59, 0x63, 0xAD, 0xCA, 0x7F, 0xE3, 0x4E, 0x03, 0x5C, 0x4E,
    0x49, 0x50, 0x47, 0x9A, 0x37, 0x61, 0xE7, 0xE2, 0xC6, 0x2E, 0x75,
    0x5A, 0xED, 0x04, 0x3D, 0x02, 0x4B, 0x78, 0x32, 0xFF, 0x58, 0x3B,
    0x7C, 0xE9, 0x00, 0x94, 0xB4, 0x4A, 0x59, 0x5B, 0xFD, 0xC9, 0x29,
    0xDF, 0x35, 0x96, 0x98, 0x9E, 0x4F, 0x30, 0x32, 0x8D};

// Version 1.0 (0x91)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 Self test
constexpr std::array<uint8_t, 64> firmware_referenceV1_0{
    0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C, 0xC2, 0xD8, 0x7C,
    0x4D, 0xD9, 0x70, 0xC7, 0x73, 0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1,
    0x3E, 0x5A, 0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E, 0x64,
    0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC, 0x22, 0xBC, 0xD3, 0x72,
    0x35, 0xCD, 0xAA, 0x41, 0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E,
    0x02, 0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79};

// Version 2.0 (0x92)
// NXP Semiconductors; Rev. 3.8 - 17 September 2014; 16.1.1 Self test
constexpr std::array<uint8_t, 64> firmware_referenceV2_0{
    0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95, 0xD0, 0xE3, 0x0D,
    0x3D, 0x27, 0x89, 0x5C, 0xDE, 0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B,
    0x89, 0x82, 0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49, 0x7C,
    0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81, 0x5D, 0x48, 0x76, 0xD5,
    0x71, 0x61, 0x21, 0xA9, 0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B,
    0x6D, 0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F};

#if 0
// Clone
// Fudan Semiconductor FM17522 (0x88)
constexpr std::array<uint8_t, 64> firmware_referenceClone{
    0x00, 0xD6, 0x78, 0x8C, 0xE2, 0xAA, 0x0C, 0x18, 0x2A, 0xB8, 0x7A,
    0x7F, 0xD3, 0x6A, 0xCF, 0x0B, 0xB1, 0x37, 0x63, 0x4B, 0x69, 0xAE,
    0x91, 0xC7, 0xC3, 0x97, 0xAE, 0x77, 0xF4, 0x37, 0xD7, 0x9B, 0x7C,
    0xF5, 0x3C, 0x11, 0x8F, 0x15, 0xC3, 0xD7, 0xC1, 0x5B, 0x00, 0x2A,
    0xD0, 0x75, 0xDE, 0x9E, 0x51, 0x64, 0xAB, 0x3E, 0xE9, 0x15, 0xB5,
    0xAB, 0x56, 0x9A, 0x98, 0x82, 0x26, 0xEA, 0x2A, 0x62};
#endif

constexpr ReceiverGain receiver_gain_table[] = {
    ReceiverGain::dB18,
    ReceiverGain::dB23,
    // 010b => dB18, 011b => dB23 (seems duplicated)
    ReceiverGain::dB18,
    ReceiverGain::dB23,
    //
    ReceiverGain::dB33,
    ReceiverGain::dB38,
    ReceiverGain::dB43,
    ReceiverGain::dB48,
};

constexpr uint8_t tx_control_tx12ref{0x03};  // Tx1RFEn | Tx2RFEn
constexpr uint8_t CASCADE_TAG{0x88};

constexpr static ISO14443Command select_command_table[] = {
    ISO14443Command::SELECT_CL1,
    ISO14443Command::SELECT_CL2,
    ISO14443Command::SELECT_CL3,
};

void dump_block(const uint8_t* buf, const uint8_t len, const int16_t block = -1,
                const int8_t sector = -1, const uint8_t ab = 0xFF,
                const bool aberror = false) {
    char tmp[128 + 1] = "   ";
    uint32_t left{};
    // Sector
    if (sector >= 0) {
        left = snprintf(tmp, 4, "%02d)", sector);
    } else {
        left = 3;
    }
    // Block
    if (block >= 0) {
        left += snprintf(tmp + left, 7, "[%03d]:", block);
    } else {
        strcat(tmp, "      ");
        left += 6;
    }
    // Data
    for (uint8_t i = 0; i < len; ++i) {
        left += snprintf(tmp + left, 4, "%02X ", buf[i]);
    }
    // Access bits
    if (ab != 0xFF) {
        if (!aberror) {
            snprintf(tmp + left, 8, "[%d %d %d]", (ab >> 2) & 1, (ab >> 1) & 1,
                     (ab & 1));
        } else {
            strcat(tmp + left, "[ERROR]");
        }
    }
    printf("%s\n", tmp);
}

// tdata must be 3 bytes or largeer
// out must be 4 bytes or larger
bool getAccessBit(uint8_t* out, const uint8_t* tdata) {
    uint8_t c1 = (tdata[1] >> 4) & 0x0F;
    uint8_t c2 = tdata[2] & 0x0F;
    uint8_t c3 = (tdata[2] >> 4) & 0x0F;
    uint8_t i1 = (tdata[0] >> 4) & 0x0F;
    uint8_t i2 = tdata[0] & 0x0F;
    uint8_t i3 = tdata[1] & 0x0F;

    out[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
    out[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
    out[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
    out[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);

    bool error = c1 != (~i1 & 0x0F) || c2 != (~i2 & 0x0F) || c3 != (~i3 & 0x0F);
    return !error;
}

}  // namespace

using namespace m5::utility::mmh3;

namespace m5 {
namespace unit {

namespace mfrc522 {
PICCType getPICCType(const uint8_t sak) {
    // SAK bit 2(1 origin) is reserved for future use, i.e. bit 2 = 1 might give
    // a different meaning to all other SAK bits.
    if (sak & 0x02) {
        return PICCType::Unknown;
    }
    if (sak & 0x04) {
        return PICCType::NotCompleted;
    }
    if (sak & 0x20) {
        return PICCType::ISO_14443_4;
    }
    if (sak & 0x40) {
        return PICCType::ISO_18092;
    }
    switch (sak) {
        case 0x00:
            return PICCType::MIFARE_UltraLight;
        case 0x01:  // TagNPlay?
            return PICCType::MIFARE_DESFire;
        case 0x08:
            return PICCType::MIFARE_Classic_1K;
        case 0x09:
            return PICCType::MIFARE_Classic;
        case 0x10:
        case 0x11:
            return PICCType::MIFARE_Plus;
        case 0x18:
            return PICCType::MIFARE_Classic_4K;
        case 0x19:
            return PICCType::MIFARE_Classic_2K;
    }
    return PICCType::Unknown;
}

}  // namespace mfrc522

using namespace mfrc522;
using namespace mfrc522::command;

// class UnitMFRC522
const char UnitMFRC522::name[] = "UnitMFRC522";
const types::uid_t UnitMFRC522::uid{"UnitMFRC522"_mmh3};
const types::uid_t UnitMFRC522::attr{0};
const MifareKey UnitMFRC522::DEFAULT_CLASSIC_KEY_A{0xFF, 0xFF, 0xFF,
                                                   0xFF, 0xFF, 0xFF};

bool UnitMFRC522::begin() {
    if (!reset()) {
        M5_LIB_LOGE("Failed to reset");
        return false;
    }

    // Rset Tx bit rate
    if (!writeRegister8(TX_MODE_REG, 0x00) ||
        // Rset Rx bit rate
        !writeRegister8(RX_MODE_REG, 0x00) ||
        // Reset modulation width
        !writeRegister8(MOD_WIDTH_REG, 0x26) ||
        // timer starts automatically at the end of the transmission inall
        // communication modes at all speeds
        !writeRegister8(TMODE_REG, 0x80) ||
        // timer 40Khz (TAuto | TAutoRestart | prescale)
        !writeRegister8(TPRESCALER_REG, 0xA9) ||
        // reload timeer 1000 (0x03e8)
        !writeRegister8(TRELOAD_REGH, 0x03) ||
        !writeRegister8(TRELOAD_REGL, 0xE8) ||
        // forces a 100 % ASK modulation
        !writeRegister8(TX_ASK_REG, 0x40)) {
        M5_LIB_LOGE("Failed to configuration");
        return false;
    }

    // Mode
    // defines the preset value for the CRC coprocess to 6363h
    // return writeRegister8(MODE_REG, 0x29) && turnOnAntenna();
    return writeRegister8(MODE_REG, 0x3D) && turnOnAntenna();
}

void UnitMFRC522::update() {
    /* nop */
}

UnitMFRC522::result_t UnitMFRC522::calculateCRC(const uint8_t* buf,
                                                const size_t len,
                                                uint16_t& crc) {
#if 1
    uint8_t mode{};
    auto result =
        // To idle
        write_pcd_command(Command::Idle)
            // Enable CRCIRq
            .and_then(
                [this]() { return this->write_register8(DIV_IRQ_REG, 0x04); })
            // Flush FIFO
            .and_then([this]() {
                return this->write_register8(FIFO_LEVEL_REG, 0x80);
            })
            // Write data
            .and_then([this, &buf, &len]() {
                return this->write_register(FIFO_DATA_REG, buf, len);
            })
            // Calculate CRC
            .and_then(
                [this]() { return this->write_pcd_command(Command::CalcCRC); })
            // Get value to check if CRC is LSB/MSB first
            .and_then([this, &mode]() {
                return this->read_register8(MODE_REG, mode);
            });

    if (!result) {
        M5_LIB_LOGE("ERROR:%u", result.error());
        return result;
    }

    uint32_t msb_first = (mode & 0x80) ? 1 : 0;

    // Wait calculation to complete
    auto timeout_at = m5::utility::millis() + 100;
    bool done{};
    do {
        uint8_t irq{};
        if (read_register8(DIV_IRQ_REG, irq) && (irq & 0x04)) {
            done = true;
            break;
        }
        std::this_thread::yield();
    } while (!done && m5::utility::millis() <= timeout_at);
    if (!done) {
        M5_LIB_LOGE("TIMEOUT");
        return m5::stl::make_unexpected(function_error_t::TIMEOUT);
    }

    // Gets the values
    m5::types::big_uint16_t crc16{};
    result = read_register8(CRC_RESULT_REGH, crc16.u8[msb_first])
                 .and_then([this, &crc16, &msb_first]() {
                     return this->read_register8(CRC_RESULT_REGL,
                                                 crc16.u8[msb_first ^ 1]);
                 });
    if (result) {
        crc = crc16.get();
        return write_pcd_command(Command::Idle);
    }
    M5_LIB_LOGE("CRCERROR");
    return result;

#else
    // To idle
    if (!write_pcd_command(Command::Idle) ||
        // Enable CRCIRq
        !writeRegister8(DIV_IRQ_REG, 0x04) ||
        // Flush FIFO
        //! set_register_bit(FIFO_LEVEL_REG, 0x80) ||
        !writeRegister8(FIFO_LEVEL_REG, 0x80) ||
        // Write data
        !writeRegister(FIFO_DATA_REG, buf, len) ||
        // Calculate CRC
        !write_pcd_command(Command::CalcCRC)) {
        return m5::stl::make_unexpected(function_error_t::ERROR);
    }

    // MSG/LSB first?
    uint8_t mode{};
    if (!readRegister8(MODE_REG, mode, 0)) {
        return m5::stl::make_unexpected(function_error_t::ERROR);
    }
    uint32_t msb_first = (mode & 0x80) ? 1 : 0;

    // Wait calculation to complete
    auto timeout_at = m5::utility::millis() + 100;
    bool done{};
    do {
        uint8_t irq{};
        if (readRegister8(DIV_IRQ_REG, irq, 0) && (irq & 0x04)) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (!done && m5::utility::millis() <= timeout_at);

    m5::types::big_uint16_t crc16{};
    if (!readRegister8(CRC_RESULT_REGH, crc16.u8[msb_first], 0) ||
        !readRegister8(CRC_RESULT_REGL, crc16.u8[msb_first ^ 1], 0)) {
        return m5::stl::make_unexpected(function_error_t::TIMEOUT);
    }
    crc = crc16.get();

    return write_pcd_command(Command::Idle);
#endif
}

bool UnitMFRC522::enablePowerDownMode() {
    CommandReg cr{};
    return (bool)read_register8(COMMAND_REG, cr.value).and_then([this, &cr]() {
        cr.powerOff(true);
        cr.command(Command::NoCmdChange);
        return this->write_register8(COMMAND_REG, cr.value);
    });
}

bool UnitMFRC522::disablePowerDownMode() {
    CommandReg cr{};
    return (bool)read_register8(COMMAND_REG, cr.value)
        .and_then([this, &cr]() {
            cr.powerOff(false);
            cr.command(Command::NoCmdChange);
            return write_register8(COMMAND_REG, cr.value);
        })
        .and_then([this]() {
            // Wait until power-down bit has been cleared (1024 clock
            // It takes 1024 clocks until the Soft power-downmode is exited
            // indicated by the PowerDown bit
            auto timeout_at = m5::utility::millis() + 1000;
            bool done{};
            do {
                uint8_t v{};
                if (this->read_register8(COMMAND_REG, v) && ((v & 0x10) == 0)) {
                    done = true;
                    break;
                }
                std::this_thread::yield();
            } while (!done && m5::utility::millis() <= timeout_at);
            return done ? result_t()
                        : m5::stl::make_unexpected(function_error_t::TIMEOUT);
        });
}

bool UnitMFRC522::reset() {
    if (write_pcd_command(Command::SoftReset)) {
        m5::utility::delay(38);  // about 37.5 ms (datasheet 8.8.2)

        // Wait the power down flag has been cleared (timeout 1sec)
        auto start_at = m5::utility::millis();
        bool done{};
        do {
            CommandReg cr{};
            if (readRegister8(COMMAND_REG, cr.value, 0) && !cr.powerOff()) {
                done = true;
                break;
            }
            m5::utility::delay(1);
        } while (m5::utility::millis() - start_at <= 1000);
        return done;
    }
    return false;
}

bool UnitMFRC522::selfTest() {
    // see also  16.1.1
    // 1. Perform a soft reset.
    if (!reset()) {
        M5_LIB_LOGE("Failed to reset");
        return false;
    }

    // 2. Clear the internal buffer by writing 25 bytes of 00h and implement the
    // Config command.
    std::array<uint8_t, 25> zero{};
    // Flush FIFO
    if (!set_register_bit(FIFO_LEVEL_REG, 0x80) ||
        // Fill FIFO with 0x00
        !writeRegister(FIFO_DATA_REG, zero.data(), zero.size()) ||
        // 2stores 25 bytes into the internal buffer
        !write_pcd_command(Command::Mem)) {
        M5_LIB_LOGE("Failed to clear");
        return false;
    }

    // 3. Enable the self test by writing 09h to the AutoTestReg register.
    // the self test is enabled by value 1001b
    if (!writeRegister8(AUTO_TEST_REG, 0x09)) {
        M5_LIB_LOGE("Failed to autitest");
        return false;
    }

    // 4. Write 00h to the FIFO buffer.
    if (!writeRegister8(FIFO_DATA_REG, 0x00)) {
        M5_LIB_LOGE("Failed to FIFO00");
        return false;
    }

    // 5. Start the self test with the CalcCRC command.
    if (!write_pcd_command(Command::CalcCRC)) {
        M5_LIB_LOGE("Failed to calcCRC");
        return false;
    }

    // 6. The self test is initiated.
    // (Wait for complete)
    auto start_at = m5::utility::millis();
    bool done{};
    do {
        uint8_t v{};
        // 0x04 : the CalcCRC command is active and all data is processed
        if (readRegister8(DIV_IRQ_REG, v, 0) && (v & 0x04)) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (m5::utility::millis() - start_at <= 1000);

    if (!done || !write_pcd_command(Command::Idle)) {
        M5_LIB_LOGE("Failed to wait");
        return false;
    }

    // 7. When the self test has completed, the FIFO buffer contains the
    // following 64 bytes:

    std::array<uint8_t, 64> buf{};
    if (!readRegister(FIFO_DATA_REG, buf.data(), buf.size(), 1)) {
        M5_LIB_LOGE("Failed to read");
        return false;
    }

    if (!writeRegister8(AUTO_TEST_REG, 0x00)) {  // To normal operation
        M5_LIB_LOGE("Failed to end");
        return false;
    }

    uint8_t ver{};
    if (!readRegister8(VERSION_REG, ver, 1)) {
        M5_LIB_LOGE("Failed to read version");
        return false;
    }

    const std::array<uint8_t, 64>* firm{};
    switch (ver) {
#if 0
    case 0x88:
            firm = &firmware_referenceClone;
            break;
#endif
        case 0x90:
            firm = &firmware_referenceV0_0;
            break;
        case 0x91:
            firm = &firmware_referenceV1_0;
            break;
        case 0x92:
            firm = &firmware_referenceV2_0;
            break;
        default:
            M5_LIB_LOGE("Unknown version %x", ver);
            return false;
    }
    return firm && (*firm == buf);  // verify
}

bool UnitMFRC522::isAntennaOn(bool& on) {
    uint8_t v{};
    on = false;
    if (readRegister8(TX_CONTROL_REG, v, 0)) {
        on = ((v & tx_control_tx12ref) == tx_control_tx12ref);
        return true;
    }
    return false;
}

bool UnitMFRC522::turnOnAntenna() {
    uint8_t v{};
    if (readRegister8(TX_CONTROL_REG, v, 0)) {
        if ((v & tx_control_tx12ref) != tx_control_tx12ref) {
            return writeRegister8(TX_CONTROL_REG, v | tx_control_tx12ref);
        }
        // Already on
        return true;
    }
    return false;
}

bool UnitMFRC522::turnOffAntenna() {
    uint8_t v{};
    if (readRegister8(TX_CONTROL_REG, v, 0)) {
        if ((v & tx_control_tx12ref)) {
            return writeRegister8(TX_CONTROL_REG, (v & ~tx_control_tx12ref));
        }
        // Already off
        return true;
    }
    return false;
}

bool UnitMFRC522::getAntennaGain(ReceiverGain& gain) {
    uint8_t v{};
    if (readRegister8(RFC_FG_REG, v, 0)) {
        gain = receiver_gain_table[(v >> 4) & 0x07];
        return true;
    }
    return false;
}

bool UnitMFRC522::setAntennaGain(const ReceiverGain gain) {
    uint8_t v{};
    if (readRegister8(RFC_FG_REG, v, 0)) {
        v = (v & ~(0x07 << 4)) | ((m5::stl::to_underlying(gain) & 0x07) << 4);
        return writeRegister8(RFC_FG_REG, v);
    }
    return false;
}

UnitMFRC522::result_t UnitMFRC522::getLatestErrorStatus(Error& err) {
    return read_register8(ERROR_REG, err.value);
}

UnitMFRC522::result_t UnitMFRC522::piccActivate(UID& uid, const bool specific) {
    uint8_t ATQA[2]{};
    uint8_t alen{sizeof(ATQA)};

    // Reset Tx bit rate
    auto result = write_register8(TX_MODE_REG, 0x00)
                      .and_then([this]() {
                          // Rset Rx bit rate
                          return this->write_register8(RX_MODE_REG, 0x00);
                      })
                      .and_then([this]() {
                          // Reset modulation width
                          return this->write_register8(MOD_WIDTH_REG, 0x26);
                      });
    if (!result) {
        return result;
    }

    bool ready{};

    // REQA for IDLE
    result = piccREQA(ATQA, alen);
    ready  = (bool)result || result.error() == function_error_t::COLLISION;
    if (!ready) {
        // WUPA for HALT
        result = piccWUPA(ATQA, alen);
        ready  = (bool)result || result.error() == function_error_t::COLLISION;
    }
    return ready ? piccSelect(uid, specific) : result;
}

UnitMFRC522::result_t UnitMFRC522::executeCommand(
    const mfrc522::Command cmd, const uint8_t waitIRQ, const uint8_t* sendData,
    const uint8_t sendLen, uint8_t* backData, uint8_t* backLen,
    uint8_t* validBits, const uint8_t rxAlign, const bool checkCRC) {
    uint8_t _validBits{};

    // Prepare values for BitFramingReg
    uint8_t txLastBit =
        validBits
            ? *validBits  // used for transmission of bit oriented frames
            : 0U;  // 0 means all bits of the last byte will be transmitted
    uint8_t bitFraming = (rxAlign << 4) + txLastBit;

    auto result =
        // To Idle
        write_pcd_command(Command::Idle)
            .and_then([this]() {
                // Enable all IRQ
                return this->write_register8(COM_IRQ_REG, 0x7F);
            })
            .and_then([this]() {
                // Flush FIFO
                return write_register8(FIFO_LEVEL_REG, 0x80);
            })
            .and_then([this, &sendData, &sendLen]() {
                // Write data to FIFO
                return this->write_register(FIFO_DATA_REG, sendData, sendLen);
            })
            .and_then([this, &bitFraming]() {
                // Adjustments for bit-oriented frames
                return write_register8(BIT_FRAMING_REG, bitFraming);
            })
            .and_then([this, &cmd]() {
                // Execute command
                return write_pcd_command(cmd);
            });
    if (!result) {
        return result;
    }
    // Starts the transmission of data if Transceive
    if (cmd == Command::Transceive) {
        result = set_register_bit(BIT_FRAMING_REG, 0x80);
        if (!result) {
            return result;
        }
    }

    // Wait for completed
    auto timeout_at = m5::utility::millis() + 50;
    bool done{};
    uint8_t irq{};
    do {
        if (readRegister8(COM_IRQ_REG, irq, 0) && (irq & waitIRQ)) {
            done = true;
            break;
        }
        // Timeout?
        if (irq & 1) {
            break;
        }
        std::this_thread::yield();
    } while (!done && m5::utility::millis() <= timeout_at);
    if (!done) {
        if (waitIRQ != 0x30) {
            M5_LIB_LOGE("Timeout occurred:%d", irq & 1);
        }
        return m5::stl::make_unexpected(function_error_t::TIMEOUT);
    }

    // Failed if deteced errors (protocol, parity, overflow)
    Error err{};
    result = getLatestErrorStatus(err);
    if (!result) {
        return result;
    }
    if (err.value & 0x13) {
        M5_LIB_LOGE(">>>>: %x: %02x%02x%02x", err.value, sendData[0],
                    sendData[1], sendData[2]);
        return m5::stl::make_unexpected(function_error_t::ERROR);
    }

    // If the caller wants data back, get it from the MFRC522
    if (backData && backLen) {
        uint8_t len{};
        result =
            // Get length
            read_register8(FIFO_LEVEL_REG, len)
                .and_then([this, &backData, &backLen, &len, &rxAlign]() {
                    // Read FIFO
                    if (*backLen < len) {
                        M5_LIB_LOGE("backLen is not large enough %u / %u", len,
                                    *backLen);
                        return result_t(
                            m5::stl::make_unexpected(function_error_t::ARG));
                    }
                    *backLen = len;
                    return this->read_register_rxAlign(FIFO_DATA_REG, backData,
                                                       len, rxAlign);
                })
                .and_then([this, &_validBits]() {
                    // Is last received byte valid?
                    return this->read_register8(CONTROL_REG, _validBits);
                });
        if (!result) {
            return result;
        }
        _validBits &= 0x07;
        if (validBits) {
            *validBits = _validBits;
        }
    }

    // Collision error?
    if (err.collision()) {
        return m5::stl::make_unexpected(function_error_t::COLLISION);
    }

    // CRC validation
    if (backData && backLen && checkCRC) {
        // In this case a MIFARE Classic NAK is not OK.
        if (*backLen == 1 && _validBits == 4) {
            return m5::stl::make_unexpected(function_error_t::NACK);
        }
        if (*backLen < 2 || _validBits != 0) {
            return m5::stl::make_unexpected(function_error_t::CRC);
        }
        // validation
        uint16_t crc16{};
        result =
            calculateCRC(&backData[0], *backLen - 2, crc16)
                .and_then([this, &backData, &backLen, &crc16]() {
                    return (backData[*backLen - 1] == ((crc16 >> 8) & 0xFF) &&
                            backData[*backLen - 2] == (crc16 & 0xFF))
                               ? result_t()
                               : m5::stl::make_unexpected(
                                     function_error_t::CRC);
                });
    }
    return {};
}

//
UnitMFRC522::result_t UnitMFRC522::piccSelect(mfrc522::UID& uid,
                                              const bool specific) {
    // Clear ValuesAfterColl
    auto result = mask_register_bit(COLL_REG, 0x80);
    if (!result) {
        return result;
    }

    bool validUID = (uid.size == 4 || uid.size == 7 || uid.size == 10);
    uint8_t cascadeLevel{0};  // 0 origin (max 2 : Lv3)
    uint8_t res[5]{};
    uint8_t rlen{5};
    uint8_t sak[3]{};
    uint8_t slen{3};

    if (validUID) {
        // last or first?????
        std::memcpy(res + (uid.size != 4), uid.uid, (uid.size == 4) ? 4 : 3);
        if (uid.size != 4) {
            res[0] = CASCADE_TAG;
        }
    }

    uint8_t collPos{0};
    // cascade loop
    do {
        bool collision{};
        // The maximum number of anti-collision attempts is 32.
        int_fast8_t count{32};

        // anti collision loop
        if (!validUID) {
            auto result = anti_collision(cascadeLevel, res, rlen, collPos);
            if (result.has_error() &&
                result.error() != function_error_t::COLLISION) {
                return result;
            }
            if (result.has_error() &&
                result.error() != function_error_t::COLLISION) {
                M5_LIB_LOGE(">>>>>>>>>>>>> COLLISION");

                uint8_t coll{};
                result = read_register8(COLL_REG, coll);
                if (result && coll & 0x20) {
                    return m5::stl::make_unexpected(
                        function_error_t::COLLISION);
                }
                if (!result) {
                    return result;
                }

                coll &= 0x1F;  // 5bits
                collPos = coll == 0 ? 32 : coll;
            }
        }

        auto result = select(cascadeLevel, res, rlen, sak, slen);
        if (result) {
            uint8_t txLastBit{0};  // TODO from select
            if (slen != 3 || txLastBit) {
                return m5::stl::make_unexpected(function_error_t::ERROR);
            }

            // M5_DUMPE(sak, sizeof(sak));
            //  M5_DUMPE(res, rlen);

            // Copy the finalised part
            std::memcpy(uid.uid + cascadeLevel * 3,
                        res + (res[0] == CASCADE_TAG),
                        (res[0] == CASCADE_TAG) ? 3 : 4);

            uint16_t crc{};
            if (!calculateCRC(sak, 1, crc) ||
                ((crc & 0xFF) != sak[1] || ((crc >> 8) & 0xFF) != sak[2])) {
                return m5::stl::make_unexpected(function_error_t::CRC);
            }

            // Complete?
            if ((sak[0] & 0x04) == 0) {
                uid.sak  = sak[0];
                uid.size = (cascadeLevel + 1) * 3 + 1;
                uid.type = getPICCType(uid.sak);
                M5_LIB_LOGI("COMPLETED:%u:%x:%x", uid.size, uid.sak, uid.type);
                return result_t();
            }
            // Not completed
            // so incrase cascade level and restart anti collision loop
        } else {
            M5_LIB_LOGE("select error:%u", result.error());
        }
        M5_LIB_LOGE("Failed select:%d:SAK:%u %x", cascadeLevel, slen, sak[0]);
    } while (++cascadeLevel < 3);

    return m5::stl::make_unexpected(function_error_t::ERROR);
}

UnitMFRC522::result_t UnitMFRC522::anti_collision(const uint8_t clv,
                                                  uint8_t* res, uint8_t& rlen,
                                                  const uint8_t collPos) {
    assert(clv < 3 && "Invalid cascade level");

    uint8_t buf[9]{m5::stl::to_underlying(select_command_table[clv]),
                   (uint8_t)(collPos + 0x20)};
    uint8_t txLastBit{0};
    uint8_t rxAlign = txLastBit;

    if (collPos) {
        // TODO: copy UID x n?
    }
    return write_register8(BIT_FRAMING_REG, (rxAlign << 4) + txLastBit)
        .and_then([this, &buf, &res, &rlen, &txLastBit, &rxAlign]() {
            return this->transceiveData(buf, 2U, res, &rlen, &txLastBit,
                                        rxAlign);
        });
}

UnitMFRC522::result_t UnitMFRC522::select(const uint8_t clv, const uint8_t* uid,
                                          const uint8_t len, uint8_t* res,
                                          uint8_t& rlen) {
    assert(len != 4 && "Invalid length");

    uint8_t buf[9]{m5::stl::to_underlying(select_command_table[clv]),
                   0x70 /* SELECT */};
    uint16_t crc{};
    uint8_t txLastBit{0};
    uint8_t rxAlign = txLastBit;

    memcpy(buf + 2, uid, 4);                     // copy UID
    buf[6] = buf[2] ^ buf[3] ^ buf[4] ^ buf[5];  // BCC

    return calculateCRC(buf, 7, crc)
        .and_then([this, &crc, &buf, &rxAlign, &txLastBit]() {
            buf[7] = crc & 0xFF;
            buf[8] = (crc >> 8) & 0xFF;
            return write_register8(BIT_FRAMING_REG, (rxAlign << 4) + txLastBit);
        })
        .and_then([this, &buf, &res, &rlen, &rxAlign, &txLastBit]() {
            return transceiveData(buf, sizeof(buf), res, &rlen, &txLastBit,
                                  rxAlign);
        });
}

UnitMFRC522::result_t UnitMFRC522::piccHLTA() {
    uint8_t cmd[4] = {m5::stl::to_underlying(ISO14443Command::HLTA), 0x00};
    uint16_t crc{};
    return calculateCRC(cmd, 2, crc).and_then([this, &crc, &cmd]() {
        cmd[2]      = crc & 0xFF;
        cmd[3]      = (crc >> 8) & 0xFF;
        auto result = transceiveData(cmd, 4);
        // HLT will not return response,so timeout means completed.
        return result ? m5::stl::make_unexpected(function_error_t::ERROR)
               : result.error() == function_error_t::TIMEOUT ? result_t()
                                                             : result;
    });
}

UnitMFRC522::result_t UnitMFRC522::picc_authenticate(const ISO14443Command cmd,
                                                     const UID& uid,
                                                     const MifareKey& key,
                                                     const uint8_t block) {
    if (cmd != ISO14443Command::AUTH_WITH_KEY_A &&
        cmd != ISO14443Command::AUTH_WITH_KEY_B) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }
    // 10.3.1.9
    uint8_t buf[12]{m5::stl::to_underlying(cmd), block};
    std::memcpy(buf + 2, key.data(), key.size());
    std::memcpy(buf + 8, uid.uid + (uid.size - 4), 4);  // UID last 4bytes

    return executeCommand(Command::MFAuthent, 0x10 /*Idle */, buf, 12);
}

UnitMFRC522::result_t UnitMFRC522::stopCrypto1() {
    // Clear MFCrypto1On
    return mask_register_bit(STATUS2_REG, 0x08);
}

UnitMFRC522::result_t UnitMFRC522::mifareRead(const uint8_t addr, uint8_t* buf,
                                              uint8_t& len) {
    if (!buf || len < 18) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }

    uint8_t cmd[4]{m5::stl::to_underlying(ISO14443Command::READ), addr};
    uint16_t crc{};
    return calculateCRC(cmd, 2, crc).and_then([this, &crc, &cmd, &buf, &len]() {
        cmd[2] = crc & 0xFF;
        cmd[3] = (crc >> 8) & 0xFF;
        return transceiveData(cmd, 4, buf, &len, nullptr, 0, true);
    });
}

UnitMFRC522::result_t UnitMFRC522::mifareWrite(const uint8_t addr,
                                               const uint8_t* buf,
                                               const uint8_t len) {
    if (!buf || len < 16) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }
    uint8_t cmd[2]{m5::stl::to_underlying(ISO14443Command::WRITE), addr};
    // Send write command with address
    return mifareTransceive(cmd, 2).and_then(
        // Send Data
        [this, &buf, &len]() { return mifareTransceive(buf, len); });
}

UnitMFRC522::result_t UnitMFRC522::mifareUltralightWrite(const uint8_t page,
                                                         const uint8_t* buf,
                                                         const uint8_t len) {
    if (!buf || len < 4) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }
    // page and data
    uint8_t cmd[6]{m5::stl::to_underlying(ISO14443Command::WRITE_UL),
                   page,
                   buf[0],
                   buf[1],
                   buf[2],
                   buf[3]};
    // M5_DUMPE(cmd, 6);
    return mifareTransceive(cmd, sizeof(cmd));
}

UnitMFRC522::result_t UnitMFRC522::mifareTransceive(const uint8_t* buf,
                                                    const uint8_t len,
                                                    const bool ignoreTimeout) {
    if (!buf || len > 16) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }
    uint8_t cmd[16 + 2 /*CRC*/]{};
    std::memcpy(cmd, buf, len);
    uint16_t crc{};
    return calculateCRC(cmd, len, crc)
        .and_then([this, &len, &crc, &cmd, &ignoreTimeout]() {
            cmd[len]     = crc & 0xFF;
            cmd[len + 1] = ((crc >> 8) & 0xFF);

            uint8_t rbuf[1]{};
            uint8_t rlen{1};
            uint8_t vbits{};

            // M5_DUMPE(cmd, len + 2);

            auto result = transceiveData(cmd, len + 2, rbuf, &rlen, &vbits);
            if (ignoreTimeout &&
                (result.has_error() &&
                 result.error() == function_error_t::TIMEOUT)) {
                // Timeout is deemed a success if ignoreTimeout is true
                return result_t();
            }
            if (!result) {
                return result;
            }
            // Check 4bitACK
            if (rlen != 1 || vbits != 4) {
                return result_t(
                    m5::stl::make_unexpected(function_error_t::ERROR));
            }
            if (rbuf[0] != 0x0A) {
                return result_t(
                    m5::stl::make_unexpected(function_error_t::NACK));
            }
            return result;
        });
}

//
void UnitMFRC522::dump(const UID& uid) {
    switch (uid.type) {
        case PICCType::MIFARE_Classic:
        case PICCType::MIFARE_Classic_1K:
        case PICCType::MIFARE_Classic_4K:
            dumpMifareClassic(uid);
            break;
        case PICCType::MIFARE_UltraLight:
            dumpMifareUltralight();
        default:
            break;
    }
    piccHLTA();
}

void UnitMFRC522::dumpMifareClassic(const UID& uid, const MifareKey& key) {
    uint8_t sectors{};
    switch (uid.type) {
        case PICCType::MIFARE_Classic:
            sectors = 5;
            break;
        case PICCType::MIFARE_Classic_1K:
            sectors = 16;
            break;
        case PICCType::MIFARE_Classic_4K:
            sectors = 40;
            break;
        default:
            return;
    }
    puts(
        "Sec  Blk:00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F "
        "[Access]\n"
        "----------------------------------------------------------------");
    for (int8_t i = sectors - 1; i >= 0; --i) {
        dump_mifare_classic_sector(uid, key, i);
    }
    piccHLTA();
    stopCrypto1();
}

void UnitMFRC522::dumpMifareUltralight() {
    uint8_t buf[16 + 2 /*CRC*/]{};
    uint8_t blen{18};

    puts(
        "Page:00 01 02 03"
        "----------------");

    for (uint8_t page = 0; page < 16; page += 4) {
        if (!mifareRead(page, buf, blen).and_then([&buf, &page]() {
                for (uint8_t offset = 0; offset < 4; ++offset) {
                    uint8_t p = page + offset;
                    printf("[%02d]:%02X %02X %02X %02X\n", p,
                           buf[offset * 4 + 0], buf[offset * 4 + 1],
                           buf[offset * 4 + 2], buf[offset * 4 + 3]);
                }
                return result_t();
            })) {
            break;
        }
    }
}

//
UnitMFRC522::result_t UnitMFRC522::read_register8(const uint8_t reg,
                                                  uint8_t& ret) {
    return readRegister8(reg, ret, 0)
               ? result_t()
               : m5::stl::make_unexpected(function_error_t::I2C);
}

UnitMFRC522::result_t UnitMFRC522::read_register(const uint8_t reg,
                                                 uint8_t* buf,
                                                 const size_t len) {
    return readRegister(reg, buf, len, 0)
               ? result_t()
               : m5::stl::make_unexpected(function_error_t::I2C);
}

UnitMFRC522::result_t UnitMFRC522::read_register_rxAlign(
    const uint8_t reg, uint8_t* buf, const size_t len, const uint8_t rxAlign) {
    return readRegister(reg, buf, len, 0)
               ? result_t()
               : m5::stl::make_unexpected(function_error_t::I2C);

#if 0
    uint8_t tmp[len + 1]{};
    auto result = read_register(reg, tmp, len + 1);
    //    buf [in,out] ???
    if (result) {
        uint8_t mask = (0xFFU << rxAlign) & 0xFF;
                        buf[0] = 
                        
        buf[0] = ((buf[0] & mask) | (buf[0] & ~mask)) >> rxAlign;
    }
    return result;
#endif
}

UnitMFRC522::result_t UnitMFRC522::write_register8(const uint8_t reg,
                                                   const uint8_t value) {
    return writeRegister8(reg, value)
               ? result_t()
               : m5::stl::make_unexpected(function_error_t::I2C);
}

UnitMFRC522::result_t UnitMFRC522::write_register(const uint8_t reg,
                                                  const uint8_t* buf,
                                                  const size_t len) {
    return writeRegister(reg, buf, len)
               ? result_t()
               : m5::stl::make_unexpected(function_error_t::I2C);
}

UnitMFRC522::result_t UnitMFRC522::set_register_bit(const uint8_t reg,
                                                    const uint8_t bit) {
    uint8_t v{};
    return read_register8(reg, v).and_then([this, &reg, &v, &bit]() {
        return this->write_register8(reg, v | bit);
    });
}

UnitMFRC522::result_t UnitMFRC522::mask_register_bit(const uint8_t reg,
                                                     const uint8_t bit) {
    uint8_t v{};
    return read_register8(reg, v).and_then([this, &reg, &v, &bit]() {
        return this->write_register8(reg, v & ~bit);
    });
}

UnitMFRC522::result_t UnitMFRC522::write_pcd_command(const Command cmd) {
    CommandReg cr;
    cr.command(cmd);
    return write_register8(COMMAND_REG, cr.value);
}

UnitMFRC522::result_t UnitMFRC522::write_picc_command_short_frame(
    const ISO14443Command piccCmd, uint8_t* ATQA, uint8_t& len) {
    if (!ATQA || len < 2) {  // The ATQA response is 2 bytes long.
        return m5::stl::make_unexpected(function_error_t::ARG);
    }

    uint8_t vbit{0x07};
    uint8_t cmd{m5::stl::to_underlying(piccCmd)};
    auto result =
        mask_register_bit(COLL_REG, 0x80)
            .and_then([this, &cmd, &ATQA, &len, &vbit]() {
                return this->transceiveData(&cmd, 1U, ATQA, &len, &vbit);
            });

    return result ? (len == 2 && !vbit)
                        ? result_t()
                        : m5::stl::make_unexpected(function_error_t::ERROR)
                  : result;
}

// RATS (Request Answer To Select)
// Only check if the ATS command is valid
bool UnitMFRC522::exists_RATS(bool& available) {
    uint8_t cmd[4] = {0xE0 /* RATS */, 0x50 /* FSD=64, CID=0 */};
    uint16_t crc{};
    return (bool)calculateCRC(cmd, 2, crc).and_then([this, &crc, &cmd]() {
        cmd[2] = crc & 0xFF;
        cmd[3] = (crc >> 8) & 0xFF;

        uint8_t res[64];
        uint8_t rlen{64};
        return transceiveData(cmd, sizeof(cmd), res, &rlen, nullptr, 0,
                              true /*CRC*/);
    });
}

UnitMFRC522::result_t UnitMFRC522::dump_mifare_classic_sector(
    const UID& uid, const MifareKey& key, const uint8_t sector) {
    if (sector >= 40) {
        return m5::stl::make_unexpected(function_error_t::ARG);
    }

    // Sector 0~31 has 4 blocks, 32-39 has 16 blocks
    const uint8_t blocks = (sector < 32) ? 4U : 16U;
    const uint8_t first =
        (sector < 32) ? sector * blocks : 128U + (sector - 32) * blocks;

    uint8_t buf[16 + 2 /*CRC*/];
    uint8_t blen{18};
    int8_t offset = blocks - 1;
    uint8_t addr  = first + offset;
    uint8_t vboff = blocks == 4 ? offset : offset / 5;

    // Sector trailer
    auto result = piccAuthenticateWithKeyA(uid, key, first)
                      .and_then([this, &addr, &buf, &blen]() {
                          return mifareRead(addr, buf, blen);
                      });
    if (!result) {
        return result;
    }
    uint8_t abits[4]{};
    bool error = !getAccessBit(abits, buf + 6 /* Access bit offset */);
    dump_block(buf, 16, addr, sector, abits[vboff], error);

    // Data
    for (offset = blocks - 2; offset >= 0; --offset) {
        addr = first + offset;
        if (!mifareRead(addr, buf, blen)) {
            break;
        }
        uint8_t vboff = (blocks == 4) ? offset : offset / 5;
        bool fig =
            (blocks == 4) ? true : (vboff == 3) || (vboff != (offset + 1) / 5);

        dump_block(buf, 16, addr, -1, fig ? abits[vboff] : 0xFF, error);

        if (vboff != 3 &&
            (abits[vboff] == 1 ||
             abits[vboff] == 6)) {  // Not a sector trailer, a value block
            int32_t value = (int32_t(buf[3]) << 24) | (int32_t(buf[2]) << 16) |
                            (int32_t(buf[1]) << 8) | int32_t(buf[0]);

#if 0
            Serial.print(F(" Value=0x"));
            Serial.print(value, HEX);
            Serial.print(F(" Adr=0x"));
            Serial.print(buffer[12], HEX);
#endif
            // TODO: KEY-B is not key (any value)
        }
    }
    return {};
}

}  // namespace unit
}  // namespace m5
