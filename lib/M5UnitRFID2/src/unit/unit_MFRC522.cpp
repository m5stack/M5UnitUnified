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

constexpr static PICCCommand select_command_table[] = {
    PICCCommand::SELECT_CL1,
    PICCCommand::SELECT_CL2,
    PICCCommand::SELECT_CL3,
};

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

bool UnitMFRC522::calculateCRC(const uint8_t* buf, const size_t len,
                               uint16_t& result) {
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
        return false;
    }

    // MSG/LSB first?
    uint8_t mode{};
    if (!readRegister8(MODE_REG, mode, 0)) {
        return false;
    }
    uint32_t msb_first = (mode & 0x80) ? 1 : 0;

    // Wait calculation to complete
    auto start_at = m5::utility::millis();
    bool done{};
    do {
        uint8_t irq{};
        if (readRegister8(DIV_IRQ_REG, irq, 0) && (irq & 0x04)) {
            done = true;
            break;
        }
        m5::utility::delay(1);
    } while (!done && m5::utility::millis() - start_at <= 100);

    m5::types::big_uint16_t crc{};
    if (!readRegister8(CRC_RESULT_REGH, crc.u8[msb_first], 0) ||
        !readRegister8(CRC_RESULT_REGL, crc.u8[msb_first ^ 1], 0)) {
        return false;
    }
    result = crc.get();

    return write_pcd_command(Command::Idle);
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

bool UnitMFRC522::getLatestErrorStatus(Error& err) {
    return readRegister8(ERROR_REG, err.value, 0);
}

bool UnitMFRC522::activate(UID& uid, const bool specific) {
    uint8_t ATOA[2]{};
    uint8_t ATOA2[2]{};
    uint8_t len{sizeof(ATOA)};
    uint8_t len2{sizeof(ATOA)};

    // REQ for IDLE, WUPA for HALT
    // To READY if successful
    if (commandREQA(ATOA, len) || commandWUPA(ATOA2, len2)) {
        // To ACTIVE if successful
        return commandSelect(uid, specific);
    }
    return false;
}

bool UnitMFRC522::executeCommand(Error& err, const mfrc522::Command cmd,
                                 const uint8_t waitIRQ, const uint8_t* sendData,
                                 const uint8_t sendLen, uint8_t* backData,
                                 uint8_t* backLen, uint8_t* validBits,
                                 const uint8_t rxAlign, const bool checkCRC) {
    err.value = 0;
    uint8_t _validBits{};

    // Prepare values for BitFramingReg
    uint8_t txLastBit  = validBits ? *validBits : 0U;
    uint8_t bitFraming = (rxAlign << 4) + txLastBit;

    // To idle
    if (!write_pcd_command(Command::Idle) ||
        // Enable all IRQ
        !writeRegister8(COM_IRQ_REG, 0x7F) ||
        // Flush FIFO
        //! set_register_bit(FIFO_LEVEL_REG, 0x80) ||
        !writeRegister8(FIFO_LEVEL_REG, 0x80) ||
        // Write data to FIFO
        !writeRegister(FIFO_DATA_REG, sendData, sendLen) ||
        // Adjustments for bit-oriented frames
        !writeRegister8(BIT_FRAMING_REG, bitFraming) ||
        // Execute command
        !write_pcd_command(cmd)) {
        M5_LIB_LOGE("Failed to execute %x", cmd);
        return false;
    }
    // Starts the transmission of data if Transceive
    if (cmd == Command::Transceive) {
        uint8_t v{};
        if (!readRegister8(BIT_FRAMING_REG, v, 0) ||
            !writeRegister8(BIT_FRAMING_REG, (v | 0x80))) {
            M5_LIB_LOGE("Failed to transceive");
        }
    }

    // Wait for complete (timeout 100ms)
    auto start_at = m5::utility::millis();
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
    } while (!done && m5::utility::millis() - start_at <= 40);
    if (!done) {
        if (waitIRQ != 0x30) {
            M5_LIB_LOGE("Timeout occurred:%d", irq & 1);
        }
        return false;
    }

    // Failed if deteced errors (protocol, parity, overflow)
    if (!getLatestErrorStatus(err) || (err.value & 0x13)) {
        M5_LIB_LOGE("Error occurred %x", err.value);
        return false;
    }

    // If the caller wants data back, get it from the MFRC522
    if (backData && backLen) {
        uint8_t len{};
        if (readRegister8(FIFO_LEVEL_REG, len, 0)) {
            if (*backLen < len) {
                M5_LIB_LOGE("backLen is not large enough %u / %u", len,
                            *backLen);
                return false;
            }
            if (len) {
                *backLen = len;
                // Read FIFO
                if (!readRegister(FIFO_DATA_REG, backData, len, 0)) {
                    M5_LIB_LOGE("Failed to read");
                    return false;
                }
                // Is last received byte valid?
                if (!readRegister8(CONTROL_REG, _validBits, 0) ||
                    (_validBits & 0x07)) {
                    M5_LIB_LOGE("Failed to read control reg");
                    return false;
                }
                if (validBits) {
                    *validBits = _validBits;
                }
            }
        }
    }

    // Collision error?
    if (err.collision()) {
        M5_LIB_LOGE("Collision Error occurred %x", err.value);
        return false;
    }

    // CRC validation
    if (backData && backLen && checkCRC) {
        // In this case a MIFARE Classic NAK is not OK.
        if (*backLen == 1 && _validBits == 4) {
            M5_LIB_LOGE("NAK");
            return false;
        }
        // We need at least the CRC_A value and all 8 bits of the last
        // uint8_t must be received.
        if (*backLen < 2 || _validBits != 0) {
            M5_LIB_LOGE("CRC");
            return false;
        }
        // Verify CRC_A - do our own calculation and store the control in
        // controlBuffer.
        uint16_t crc16{};
        if (!calculateCRC(&backData[0], *backLen - 2, crc16) ||
            (backData[*backLen - 2] != ((crc16 >> 8) & 0xFF) ||
             backData[*backLen - 1] != (crc16 & 0xFF))) {
            M5_LIB_LOGE("CRC error %x", crc16);
            return false;
        }
    }
    return true;
}

//
bool UnitMFRC522::commandSelect(mfrc522::UID& uid, const bool specific) {
    // Clear ValuesAfterColl
    if (!mask_register_bit(COLL_REG, 0x80)) {
        return false;
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

    // cascade loop
    do {
        bool collision{};
        // The maximum number of anti-collision attempts is 32.
        int_fast8_t count{32};

        // anti collision loop
        if (!validUID) {
            do {
                if (!anti_collision(cascadeLevel, collision, res, rlen)) {
                    M5_LIB_LOGE("Failed to anti_collision:%d", collision);
                    return false;
                }
            } while (collision && count--);

            if (count < 0) {
                return false;
            }

            if (collision) {
                M5_LIB_LOGE(">>>>>>>>>>>>> COLLISION");
                return false;
            }
        }

        if (select(cascadeLevel, res, rlen, sak, slen)) {
            uint8_t txLastBit{0};  // TODO from select
            if (slen != 3 || txLastBit) {
                return false;
            }

            M5_DUMPE(sak, sizeof(sak));
            // M5_DUMPE(res, rlen);

            // Copy the finalised part
            std::memcpy(uid.uid + cascadeLevel * 3,
                        res + (res[0] == CASCADE_TAG),
                        (res[0] == CASCADE_TAG) ? 3 : 4);

            uint16_t crc{};
            if (!calculateCRC(sak, 1, crc) ||
                ((crc & 0xFF) != sak[1] || ((crc >> 8) & 0xFF) != sak[2])) {
                return false;
            }

            // Complete?
            if ((sak[0] & 0x04) == 0) {
                uid.sak  = sak[0];
                uid.size = (cascadeLevel + 1) * 3 + 1;
                uid.type = getPICCType(uid.sak);
                M5_LIB_LOGI("COMPLETED:%u:%x:%x", uid.size, uid.sak, uid.type);
                return true;
            }
            // Not completed
            // so incrase cascade level and restart anti collision loop
        }
    } while (++cascadeLevel < 3);

    return false;
}

bool UnitMFRC522::anti_collision(const uint8_t clv, bool& collision,
                                 uint8_t* res, uint8_t& rlen,
                                 const uint8_t collPos) {
    assert(clv < 3 && "Invalid cascade level");

    uint8_t buf[9]{m5::stl::to_underlying(select_command_table[clv]),
                   (uint8_t)(collPos + 0x20)};
    uint8_t txLastBit{0};
    uint8_t rxAlign = txLastBit;

    if (!writeRegister8(BIT_FRAMING_REG, (rxAlign << 4) + txLastBit)) {
        M5_LIB_LOGE("Failed to bitstream");
        return false;
    }

    Error err{};
    auto result = transceiveData(err, buf, 2U, res, &rlen, &txLastBit, rxAlign);
    if (result) {
        collision = false;
        return true;
    }
    collision = err.collision();
    M5_LIB_LOGE("err:%x", err.value);
    return collision;
}

bool UnitMFRC522::select(const uint8_t clv, const uint8_t* uid,
                         const uint8_t len, uint8_t* res, uint8_t& rlen) {
    assert(len != 4 && "Invalid length");

    uint8_t buf[9]{m5::stl::to_underlying(select_command_table[clv]),
                   0x70 /* SELECT */};
    uint16_t crc{};

    memcpy(buf + 2, uid, 4);                     // copy UID
    buf[6] = buf[2] ^ buf[3] ^ buf[4] ^ buf[5];  // BCC
    if (calculateCRC(buf, 7, crc)) {
        buf[7] = crc & 0xFF;
        buf[8] = (crc >> 8) & 0xFF;
    }

    uint8_t txLastBit{0};
    uint8_t rxAlign = txLastBit;

    if (writeRegister8(BIT_FRAMING_REG, (rxAlign << 4) + txLastBit)) {
        Error err{};
        return transceiveData(err, buf, sizeof(buf), res, &rlen, &txLastBit,
                              rxAlign);
    }
    return false;
}

bool UnitMFRC522::commandAuthenticate(const PICCCommand cmd, const UID& uid,
                                      const MifareKey& key,
                                      const uint8_t block) {
    if (cmd != PICCCommand::AUTH_WITH_KEY_A &&
        cmd != PICCCommand::AUTH_WITH_KEY_B) {
        M5_LIB_LOGE("Invalid command:%x", cmd);
        return false;
    }
    // 10.3.1.9
    uint8_t buf[12]{m5::stl::to_underlying(cmd), block};
    std::memcpy(buf + 2, key.data(), key.size());
    std::memcpy(buf + 8, uid.uid + (uid.size - 4), 4);  // UID last 4bytes

    Error err{};
    return executeCommand(err, Command::MFAuthent, 0x10 /*Idle */, buf, 12);
}

bool UnitMFRC522::readMifare(const uint8_t addr, uint8_t* buf, uint8_t& len) {
    if (!buf || len < 16) {
        return false;
    }
    uint8_t cmd[4]{m5::stl::to_underlying(PICCCommand::READ), addr};
    uint16_t crc{};
    if (!calculateCRC(cmd, 2, crc)) {
        M5_LIB_LOGE("CRC");
        return false;
    }
    cmd[2] = crc & 0xFF;
    cmd[3] = (crc >> 8) & 0xFF;
    Error err{};
    if (transceiveData(err, cmd, sizeof(cmd), buf, &len, nullptr, 0, true)) {
        return true;
    }
    M5_LIB_LOGE("ERR:%x", err.value);
    return false;
}

//
void UnitMFRC522::dump(const UID& uid) {
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
            break;
    }
    if (!sectors) {
        M5_LIB_LOGE("Not supported");
        return;
    }

    MifareKey key;
    // All keys are set to FFFFFFFFFFFFh at chip delivery from the factory
    key.fill(0xFF);

    uint8_t res[128]{};
    uint8_t rlen{128};
    //    for (uint8_t i = sectors - 1; i >= 0; --i) {
    read_mifare_sector(uid, key, sectors - 1, res);
    //    }
}

//
bool UnitMFRC522::set_register_bit(const uint8_t reg, const uint8_t bit) {
    uint8_t v{};
    return readRegister8(reg, v, 0) && writeRegister8(reg, v | bit);
}
bool UnitMFRC522::mask_register_bit(const uint8_t reg, const uint8_t bit) {
    uint8_t v{};
    return readRegister8(reg, v, 0) && writeRegister8(reg, v & ~bit);
}

bool UnitMFRC522::write_pcd_command(const Command cmd) {
    CommandReg cr;
    cr.command(cmd);
    return writeRegister8(COMMAND_REG, cr.value);
}

bool UnitMFRC522::write_picc_command_short_frame(const PICCCommand piccCmd,
                                                 uint8_t* ATQA, uint8_t& len) {
    if (ATQA == NULL || len < 2) {  // The ATQA response is 2 bytes long.
        return false;
    }

    uint8_t col{};
    if (!readRegister8(COLL_REG, col, 0) ||
        !writeRegister8(COLL_REG, (col & ~0x80))) {
        return false;
    }

    uint8_t validBits{7};
    uint8_t cmd{m5::stl::to_underlying(piccCmd)};
    Error err{};
    if (!transceiveData(err, &cmd, 1U, ATQA, &len, &validBits)) {
        return false;
    }
    return (len == 2 && validBits);
}

// RATS (Request Answer To Select)
// Only check if the ATS command is valid
bool UnitMFRC522::exists_RATS(bool& available) {
    uint8_t buf[4] = {0xE0 /* RATS */, 0x50 /* FSD=64, CID=0 */};
    uint16_t crc{};
    if (!calculateCRC(buf, 2, crc)) {
        return false;
    }
    buf[2] = crc & 0xFF;
    buf[3] = (crc >> 8) & 0xFF;

    uint8_t res[64];
    uint8_t rlen{64};
    Error err{};
    available = transceiveData(err, buf, sizeof(buf), res, &rlen, nullptr, 0,
                               true /*CRC*/);
    return available;
}

bool UnitMFRC522::read_mifare_sector(const UID& uid, const MifareKey& key,
                                     const uint8_t sector, uint8_t* res) {
    if (sector >= 40) {
        M5_LIB_LOGE("Illegal sector:%u", sector);
        return false;
    }

    // Sector 0~31 has 4 blocks, 32-39 has 16 blocks
    uint8_t blocks = (sector < 32) ? 4U : 16U;
    uint8_t first =
        (sector < 32) ? sector * blocks : 128U + (sector - 32) * blocks;

    // First: sector trailer
    uint8_t addr = first + (blocks - 1);
    M5_LIB_LOGI(">>>> first:%u", first);
    if (!commandAuthenticateWithKeyA(uid, key, first)) {
        M5_LIB_LOGE("Failed to authenticate");
        return false;
    }

    uint8_t buf[18]{};
    uint8_t blen{sizeof(buf)};

    M5_LIB_LOGE("addr:%u", addr);
    if (!readMifare(addr, buf, blen)) {
        M5_LIB_LOGE("Failed to readMifare");
        return false;
    }
    M5_DUMPE(buf, blen);

    return true;
}

#if 0
    
    // Read block
    byteCount = sizeof(buffer);
    status    = MIFARE_Read(blockAddr, buffer, &byteCount);
    if (status != STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(GetStatusCodeName(status));
        continue;
    }
    // Dump data
    for (uint8_t index = 0; index < 16; index++) {
        if (buffer[index] < 0x10)
            Serial.print(F(" 0"));
        else
            Serial.print(F(" "));
        Serial.print(buffer[index], HEX);
        if ((index % 4) == 3) {
            Serial.print(F(" "));
        }
    }
    // Parse sector trailer data
    if (isSectorTrailer) {
        c1            = buffer[7] >> 4;
        c2            = buffer[8] & 0xF;
        c3            = buffer[8] >> 4;
        c1_           = buffer[6] & 0xF;
        c2_           = buffer[6] >> 4;
        c3_           = buffer[7] & 0xF;
        invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) ||
                        (c3 != (~c3_ & 0xF));
        g[0]            = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
        g[1]            = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
        g[2]            = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
        g[3]            = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
        isSectorTrailer = false;
    }

    for (int_fast8_t offset = blocks - 2; offset >= 0; --offset) {
        uitn8_t addr{first + offset};
    }
}
#endif

}  // namespace unit
}  // namespace m5
