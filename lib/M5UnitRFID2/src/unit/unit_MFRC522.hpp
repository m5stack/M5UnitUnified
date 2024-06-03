/*!
  @file unit_MFRC522.hpp
  @brief MFRC522 Unit for M5UnitUnified

  SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD

  SPDX-License-Identifier: MIT
*/
#ifndef M5_UNIT_RFID2_UNIT_MFRC522_HPP
#define M5_UNIT_RFID2_UNIT_MFRC522_HPP

#include <M5UnitComponent.hpp>
#include <m5_utility/stl/extension.hpp>
#include <array>

namespace m5 {
namespace unit {

namespace mfrc522 {

///@cond
enum class Command : uint8_t {
    Idle,
    Mem,
    GenerateRandomID,
    CalcCRC,
    Transmit,
    NoCmdChange = 0x07,
    Receive,
    Transceive = 0x0C,
    MFAuthent  = 0x0E,
    SoftReset,
};

struct CommandReg {
    //! @brief analog part of the receiver is switched off
    inline bool rcvOff() const {
        return value & (1U << 5);
    }
    //! @brief Soft power-down mode entered
    inline bool powerOff() const {
        return value & (1U << 4);
    }
    //!@brief activates a command based on the Command value; reading
    inline Command command() const {
        return static_cast<Command>(value & 0x0f);
    }

    inline void rcvOff(const bool b) {
        value = (value & ~(1U << 5)) | ((b ? 1U : 0) << 5);
    }
    inline void powerOff(const bool b) {
        value = (value & ~(1U << 4)) | ((b ? 1U : 0) << 4);
    }
    inline void command(const Command c) {
        value = (value & ~0x0F) | (m5::stl::to_underlying(c) & 0x0F);
    }
    uint8_t value{};
};
///@endcond

/*!
  @struct Error
  @brief Error status
 */
struct Error {
    //! @brief data is written into the FIFO buffer by the host during the
    //! MFAuthent command or if data is written into the FIFO buffer by the host
    //! during the time between sending the last bit on the RF interface and
    //! receiving the last bit on the RF interface
    bool write() const {
        return value & (1U << 7);
    }
    //! @brief Internal temperature sensor detects overheating
    bool overheat() const {
        return value & (1U << 6);
    }
    //!@brief Tried to write data to the FIFO buffer even though it is already
    //! full
    bool overflow() const {
        return value & (1U << 4);
    }
    //! @brief A bit-collision is detected
    bool collision() const {
        return value & (1U << 3);
    }
    //! @brief The CRC calculation failed
    bool crc() const {
        return value & (1U << 2);
    }
    //! @brief Parity check failed
    bool parity() const {
        return value & (1U << 1);
    }
    //! @brief Tthe SOF is incorrect
    bool protocol() const {
        return value & (1U << 0);
    }
    uint8_t value{};  // bit 5 reserved
};

/*!
  @enum ReceiverGain
  @brief The receiver’s signal voltage gain facor
 */
enum class ReceiverGain : uint8_t {
    dB18,  //!< @brief 18 decibel
    dB23,  //!< @brief 23 decibel
    // 2 = dB18, 3 = dB23 (duplicated)
    dB33 = 0x04,  //!< @brief 33 decibel
    dB38,         //!< @brief 38 decibel
    dB43,         //!< @brief 43 decibel
    dB48,         //!< @brief 48 decibel
};

//! @typedef m5::unit::mfrc522::MiFareKey
using MifareKey = std::array<uint8_t, 6>;  //!< @brief A Mifare Crypto1 key

/*!
  @enum PICCType
  @brief Type of the PICC
  @warning Only support MIFARE_Classic, 1K, 4K
 */
enum class PICCType : uint8_t {
    //! @brief Unknown type
    Unknown,
    //! @brief PICC compliant with ISO/IEC 14443-4
    ISO_14443_4,
    //! @brief PICC compliant with ISO/IEC 18092 (NFC)
    ISO_18092,
    //! @brief Also known as MIFARE Standard mini
    MIFARE_Classic,
    //! @brief Also known as MIFARE Standard 1K
    MIFARE_Classic_1K,
    //! @brief Also known as MIFARE Standard 4K
    MIFARE_Classic_4K,
    //! @brief Also known as MIFARE Standard 2K
    MIFARE_Classic_2K,
    //! @brief MIFARE Ultralight or Ultralight C
    MIFARE_UltraLight,
    //! @brief MIFARE Plus
    MIFARE_Plus,
    //! @brief MIFARE DESFire (Also known as TNP3XXX)
    MIFARE_DESFire,
    //! @brief SAK indicates UID is not complete
    NotCompleted = 0xFF,
};

PICCType getPICCType(const uint8_t sak);

/*!
  @struct UID
  @brief The UID of the PICC
 */
struct UID {
    //! @brief  Number of bytes in the UID. 4, 7 or 10.
    uint8_t size;
    //! @brief uid data (Valid up to the value of size)
    uint8_t uid[10];
    //! @brief  The SAK (Select acknowledge) returned from the PICC after
    //! successful selection.
    uint8_t sak;
    //! @brief PICC type
    PICCType type;
};

/*!
  @enum PICCCommand
  @brief MIFARE classic command
 */
enum class PICCCommand : uint8_t {
    REQA = 0x26,
    WUPA = 0x52,
    HLTA = 0x50,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 1)
    SELECT_CL1 = 0x93,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 2)
    SELECT_CL2 = 0x95,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 3)
    SELECT_CL3 = 0x97,

    AUTH_WITH_KEY_A       = 0x60,
    AUTH_WITH_KEY_B       = 0x61,
    PERSONALIZE_UID_USAGE = 0x40,
    SET_MOD_TYPE          = 0x43,
    READ                  = 0x30,

};

}  // namespace mfrc522

/*!
  @class UnitMFRC522
  @brief Radio frequency identification unit
  @details Operating frequency: 13.56 MHz
  @details Supported protocols: ISO14443A, MIFARE and NTAG
 */
class UnitMFRC522 : public Component {
   public:
    constexpr static uint8_t DEFAULT_ADDRESS{0x28};
    static const types::uid_t uid;
    static const types::attr_t attr;
    static const char name[];

#if 0
    /*!
      @struct config_t
      @brief Settings
     */
    struct config_t {
    };
#endif

    explicit UnitMFRC522(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr) {
    }
    virtual ~UnitMFRC522() {
    }

    virtual bool begin() override;
    virtual void update() override;

#if 0
    ///@name Settings
    ///@{
    /*! @brief Gets the configration */
    config_t config() {
        return _cfg;
    }
    //! @brief Set the configration
    void config(const config_t& cfg) {
        _cfg = cfg;
    }
    ///@}
#endif

    /*!
      @brief Gets the the error status of the last command executed
      @param err Error status
      @return True if successful
    */
    bool getLatestErrorStatus(mfrc522::Error &err);

    bool reset();
    virtual bool selfTest();

    /*!
      @brief Calculate CRC by coprocessor
      @param[in] buf Input buffer
      @param[in] len Length of buffer
      @param[out] result CRC16
      @return True if successful
      @warning The calculation algorithm is dependent on the value of MODE_REG
    */
    bool calculateCRC(const uint8_t *buf, const size_t len, uint16_t &result);

    ///@name Antenna
    ///@{
    /*!
      @brief Is antenna on?
      @param[out] status (true:ON false:OFF)
      @return True if successful
     */
    bool isAntennaOn(bool &status);
    /*!
      @brief Turn on the antenna
      @return True if successful
     */
    bool turnOnAntenna();
    /*!
      @brief Turn off the antenna
      @return True if successful
     */
    bool turnOffAntenna();
    /*!
      @brief Gets the receiver gain
      @param[out] gain Gain
      @return True if successful
     */
    bool getAntennaGain(mfrc522::ReceiverGain &gain);
    /*!
      @brief Set the receiver gain
      @param gain Gain
      @return True if successful
     */
    bool setAntennaGain(const mfrc522::ReceiverGain gain);
    ///@}

    ///@name ISO14443-3 command
    ///@{
    /*!
      @brief Transition the PICC to the Active state
      @param[in, out] uid in: UID packed with values if for a specific
      target if specific is Ture<br>
      out: UID on the activated target
      @param specific Activate the specified uid?
      @return True if successful
      @note Processing can be carried out normally in both IDLE and HALT states
    */
    bool activate(mfrc522::UID &uid, const bool specific = false);

    /*!
      @brief ISO/IEC 14443-3 REQA
      @param[out] ATOA Buffer to receive ATOA output
      @param[in,out] len in: Maximum buffer length out:Output buffer length
      @return True if successful
      @note This command is executed for RF tags in the IDLE state.
      After the process is completed, the RF tag moves to the READY1 state.
     */
    inline bool commandREQA(uint8_t *ATOA, uint8_t &len) {
        return write_picc_command_short_frame(mfrc522::PICCCommand::REQA, ATOA,
                                              len);
    }
    /*!
      @brief ISO/IEC 14443-3 WUPA
      @note This command is executed for RF tags in the IDLE or HALT state.
      After the process is completed, the RF tag transits to the READY1 or
      READY1* state.
     */
    bool commandWUPA(uint8_t *ATOA, uint8_t &len) {
        return write_picc_command_short_frame(mfrc522::PICCCommand::WUPA, ATOA,
                                              len);
    }

    /*!
      @brief ISO/IEC 14443-3 SELECT
     */
    bool commandSelect(mfrc522::UID &uid, const bool specific);

    /*!
      @brief ISO/IEC 14443-3 HLTA
      @note Valid in ACTIVE or ACTIVE* state (after select)
      After the process is completed, the RF Tag transits to the HALT state
     */
    bool commandHLTA();

    bool commandAuthenticate(const mfrc522::PICCCommand cmd,
                             const mfrc522::UID &uid,
                             const mfrc522::MifareKey &key,
                             const uint8_t block);
    inline bool commandAuthenticateWithKeyA(const mfrc522::UID &uid,
                                            const mfrc522::MifareKey &key,
                                            const uint8_t block) {
        return commandAuthenticate(mfrc522::PICCCommand::AUTH_WITH_KEY_A, uid,
                                   key, block);
    }
    inline bool commandAuthenticateWithKeyB(const mfrc522::UID &uid,
                                            const mfrc522::MifareKey &key,
                                            const uint8_t block) {
        return commandAuthenticate(mfrc522::PICCCommand::AUTH_WITH_KEY_B, uid,
                                   key, block);
    }
    ///@}

    ///@name MIFARE
    ///@{
    bool readMifare(const uint8_t addr, uint8_t *buf, uint8_t &len);
    ///@}

    ///@name For debug
    ///@{
    void dump(const mfrc522::UID &uid);
    ///@}

    ///@name PICC (Proximity IC Card)
    ///@{
    /*!
      @brief Executes command
      @param[out] err Erros status
      @param cmd The command to execute
      @param waitIRQ The bits in the COM_IRQ_REG register that signals
      successful completion of the command
      @param sendData Pointer to the data to transfer to the FIFO
      @param sendLen Number of bytes to transfer to the FIFO
      @param backData nullptr or pointer to buffer if data should be read
      back after executing the command
      @param backLen[in,out] in: Max number of bytes to write to backData<br>
      out:The number of bytes returned
      @param[in,out] validBit The number of valid bits in the last byte
      (0 for 8 valid bits)
      @param rxAlign Defines the bit position in backData[0] for the first
      bit received (Default as 0)
      @param checkCRC The last two bytes of the response is assumed to be a
      CRC-A that must be validated
      @return True if successful
      @note CRC validation can only be done if backData and backLen are
      specified
     */
    bool executeCommand(mfrc522::Error &err, const mfrc522::Command cmd,
                        const uint8_t waitIRQ, const uint8_t *sendData,
                        const uint8_t sendLen, uint8_t *backData = nullptr,
                        uint8_t *backLen   = nullptr,
                        uint8_t *validBits = nullptr, const uint8_t rxAlign = 0,
                        const bool checkCRC = false);

    //!@ brief Transceive data
    inline bool transceiveData(mfrc522::Error &err, const uint8_t *sendData,
                               const uint8_t sendLen, uint8_t *backData,
                               uint8_t *backLen, uint8_t *validBits = nullptr,
                               const uint8_t rxAlign = 0,
                               const bool checkCRC   = false) {
        return executeCommand(err, mfrc522::Command::Transceive,
                              0x30 /*Rx and Idle */, sendData, sendLen,
                              backData, backLen, validBits, rxAlign, checkCRC);
    }

   protected:
    inline virtual const char *unit_device_name() const override {
        return name;
    }
    inline virtual types::uid_t unit_identifier() const override {
        return uid;
    }
    inline virtual types::attr_t unit_attribute() const override {
        return attr;
    }

    bool set_register_bit(const uint8_t reg, const uint8_t bit);
    bool mask_register_bit(const uint8_t reg, const uint8_t bit);

    bool write_pcd_command(const mfrc522::Command cmd);
    bool write_picc_command_short_frame(const mfrc522::PICCCommand cmd,
                                        uint8_t *ATQA, uint8_t &len);

    bool anti_collision(const uint8_t clv, bool &collision, uint8_t *res,
                        uint8_t &rlen, const uint8_t collPos = 0);
    bool select(const uint8_t clv, const uint8_t *uid, const uint8_t len,
                uint8_t *res, uint8_t &rlen);

    bool exists_RATS(bool &available);

    bool read_mifare_sector(const mfrc522::UID &uid,
                            const mfrc522::MifareKey &key, const uint8_t sector,
                            uint8_t *res);
};

///@cond
namespace mfrc522 {
namespace command {
// Command and status
constexpr uint8_t COMMAND_REG{0x01};
constexpr uint8_t COM_IEN_REG{0x02};

constexpr uint8_t COM_IRQ_REG{0x04};
constexpr uint8_t DIV_IRQ_REG{0x05};
constexpr uint8_t ERROR_REG{0x06};

constexpr uint8_t FIFO_DATA_REG{0x09};
constexpr uint8_t FIFO_LEVEL_REG{0x0A};

constexpr uint8_t CONTROL_REG{0x0C};
constexpr uint8_t BIT_FRAMING_REG{0x0D};
constexpr uint8_t COLL_REG{0x0E};

// Command
constexpr uint8_t MODE_REG{0x11};
constexpr uint8_t TX_MODE_REG{0x12};
constexpr uint8_t RX_MODE_REG{0x13};

constexpr uint8_t TX_CONTROL_REG{0x14};
constexpr uint8_t TX_ASK_REG{0x15};

// Configuration
constexpr uint8_t CRC_RESULT_REGH{0x21};
constexpr uint8_t CRC_RESULT_REGL{0x22};

constexpr uint8_t MOD_WIDTH_REG{0x24};

constexpr uint8_t RFC_FG_REG{0x26};

constexpr uint8_t TMODE_REG{0x2A};
constexpr uint8_t TPRESCALER_REG{0x2B};
constexpr uint8_t TRELOAD_REGH{0x2C};
constexpr uint8_t TRELOAD_REGL{0x2D};

// Test register
constexpr uint8_t AUTO_TEST_REG{0x36};
constexpr uint8_t VERSION_REG{0x37};

}  // namespace command
}  // namespace mfrc522
///@endcond

}  // namespace unit
}  // namespace m5
#endif
