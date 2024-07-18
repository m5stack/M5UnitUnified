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
#include <m5_utility/stl/expected.hpp>
#include <array>

namespace m5 {
namespace unit {

namespace mfrc522 {

///@cond
// PCD command
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
  @brief Group of bits that identify errors
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
  @enum ISO14443Command
  @brief ISO 14443-3 compliant commands
 */
enum class ISO14443Command : uint8_t {
    REQA = 0x26,  //!< @brief Probe the field for PICCs of Type A (for IDLE)
    WUPA =
        0x52,  //!< @brief Probe the field for PICCs of Type A (For IDLE/HALT)
    HLTA = 0x50,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 1)
    SELECT_CL1 = 0x93,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 2)
    SELECT_CL2 = 0x95,
    //! @brief ANTICOLLISION / SELECT if NVB is 40 bits (cascade level 3)
    SELECT_CL3 = 0x97,

    AUTH_WITH_KEY_A = 0x60,
    AUTH_WITH_KEY_B = 0x61,
    AUTH_UL         = 0x1A,

    PERSONALIZE_UID_USAGE = 0x40,
    SET_MOD_TYPE          = 0x43,

    READ     = 0x30,
    WRITE    = 0xA0,
    WRITE_UL = 0xA2,  //!< @brief For UltraLight (write 4 bytes)

    RATS = 0x0e,
};

/*!
  @typedef MifareKey
  @brief MIFARE Key
*/
using MifareKey = std::array<uint8_t, 6>;

}  // namespace mfrc522

/*!
  @class UnitMFRC522
  @brief Radio frequency identification unit
  @details Operating frequency: 13.56 MHz
  @details Supported protocols: ISO14443A, MIFARE and NTAG
 */
class UnitMFRC522 : public Component {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitMFRC522, 0x28);

   public:
    /*!
      @enum error_r
      @brief Error status
     */
    enum class function_error_t {
        TIMEOUT = 1,  //!< @brief Timeout occurred
        COLLISION,    //!<@brief Collisions caused by multiple PICCs
        ARG,          //!<@brief Invalid arguments
        CRC,          //!< @brief CRC error occurred
        NACK,         //!< @brief PICC responded with NACK
        I2C,          //!< @brief I2C R/W error
        ERROR,        //!< @brief Misc errors
    };
    /*!
      @typedef result_t
      @brief Result of function
    */
    using result_t = m5::stl::expected<void, function_error_t>;

    //! @brief KEY A of factory default
    static const mfrc522::MifareKey DEFAULT_CLASSIC_KEY_A;

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
    virtual void update(const bool force = false) override;

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
    result_t getLatestErrorStatus(mfrc522::Error &err);

    bool enablePowerDownMode();
    bool disablePowerDownMode();

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
    result_t calculateCRC(const uint8_t *buf, const size_t len,
                          uint16_t &result);

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

    ///@name PICC communication
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
    result_t piccActivate(mfrc522::UID &uid, const bool specific = false);

    /*!
      @brief ISO/IEC 14443-3 REQA
      @param[out] ATOA Buffer to receive ATOA output
      @param[in,out] len in: Maximum buffer length out:Output buffer length
      @return True if successful
      @note This command is executed for RF tags in the IDLE state.
      After the process is completed, the RF tag moves to the READY1 state.
     */
    inline result_t piccREQA(uint8_t *ATOA, uint8_t &len) {
        return write_picc_command_short_frame(mfrc522::ISO14443Command::REQA,
                                              ATOA, len);
    }
    /*!
      @brief ISO/IEC 14443-3 WUPA
      @note This command is executed for RF tags in the IDLE or HALT state.
      After the process is completed, the RF tag transits to the READY1 or
      READY1* state.
     */
    result_t piccWUPA(uint8_t *ATOA, uint8_t &len) {
        return write_picc_command_short_frame(mfrc522::ISO14443Command::WUPA,
                                              ATOA, len);
    }

    /*!
      @brief ISO/IEC 14443-3 SELECT
     */
    result_t piccSelect(mfrc522::UID &uid, const bool specific);

    /*!
      @brief ISO/IEC 14443-3 HLTA
      @note Valid in ACTIVE or ACTIVE* state (after select)
      After the process is completed, the RF Tag transits to the HALT state
     */
    result_t piccHLTA();

    /*!
      @brief Authentication by KEY A
      @param uid PICC UID
      @param key Authentication key
      @param block Sector trailer block address
      @warning Before a new communication can be initiated, the stopCrypto1()
      must be called and the authentication state terminated
     */
    inline result_t piccAuthenticateWithKeyA(const mfrc522::UID &uid,
                                             const mfrc522::MifareKey &key,
                                             const uint8_t block) {
        return picc_authenticate(mfrc522::ISO14443Command::AUTH_WITH_KEY_A, uid,
                                 key, block);
    }
    /*!
      @brief Authentication by KEY B
      @copydetails piccAuthenticateWithKeyA
     */
    inline result_t piccAuthenticateWithKeyB(const mfrc522::UID &uid,
                                             const mfrc522::MifareKey &key,
                                             const uint8_t block) {
        return picc_authenticate(mfrc522::ISO14443Command::AUTH_WITH_KEY_B, uid,
                                 key, block);
    }
    //! @breif Exit authentication state
    result_t stopCrypto1();
    ///@}

    ///@name MIFARE
    ///@{
    /*!
      @brief Read data block
      @param addr Block address
      @param[out] buf Buffer for storing data
      @param[in,out] len in:Length of the buffer (>= 18)<br>
      out: Stored bytes
      @note Must be authenticated
      @note The last 2 bytes of the output are CRC
      @warning The buffer length must be at least 18 bytes including the CRC
      @warning Read in units of 16 bytes
    */
    result_t mifareRead(const uint8_t addr, uint8_t *buf, uint8_t &len);
    /*!
      @brief Write data block for Classic
      @param addr Block address
      @param buf Data to be written
      @param len Length of the buffer (<= 16)
      @note Must be authenticated
      @warning Some addresses are not writable.
      (See also MIFARE Specification)
    */
    result_t mifareWrite(const uint8_t addr, const uint8_t *buf,
                         const uint8_t len);
    /*!
      @brief Write data block for Ultralight
      @param page Page number
      @param buf Data to be written
      @param len Length of the buffer (<= 4)
    */
    result_t mifareUltralightWrite(const uint8_t page, const uint8_t *buf,
                                   const uint8_t len);

    //! @brief Transceiver data for MIFARE
    result_t mifareTransceive(const uint8_t *buf, const uint8_t len,
                              const bool ignoreTimeout = false);
    ///@}

    ///@name For debug
    ///@{
    void dump(const mfrc522::UID &uid);
    void dumpMifareClassic(
        const mfrc522::UID &uid,
        const mfrc522::MifareKey &key = DEFAULT_CLASSIC_KEY_A);
    void dumpMifareUltralight();
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
      @param backLen[in,out] in: Max number of bytes to write to
      backData<br> out:The number of bytes returned
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
    result_t executeCommand(const mfrc522::Command cmd, const uint8_t waitIRQ,
                            const uint8_t *sendData, const uint8_t sendLen,
                            uint8_t *backData     = nullptr,
                            uint8_t *backLen      = nullptr,
                            uint8_t *validBits    = nullptr,
                            const uint8_t rxAlign = 0,
                            const bool checkCRC   = false);

    //!@ brief Transceive data
    inline result_t transceiveData(const uint8_t *sendData,
                                   const uint8_t sendLen,
                                   uint8_t *backData     = nullptr,
                                   uint8_t *backLen      = nullptr,
                                   uint8_t *validBits    = nullptr,
                                   const uint8_t rxAlign = 0,
                                   const bool checkCRC   = false) {
        return executeCommand(mfrc522::Command::Transceive,
                              0x30 /*Rx and Idle */, sendData, sendLen,
                              backData, backLen, validBits, rxAlign, checkCRC);
    }

   protected:
    result_t read_register8(const uint8_t reg, uint8_t &ret);
    result_t read_register(const uint8_t reg, uint8_t *buf, const size_t len);
    result_t read_register_rxAlign(const uint8_t reg, uint8_t *buf,
                                   const size_t len, const uint8_t rxAlign);
    result_t write_register8(const uint8_t reg, const uint8_t value);
    result_t write_register(const uint8_t reg, const uint8_t *buf,
                            const size_t size);
    result_t set_register_bit(const uint8_t reg, const uint8_t bit);
    result_t mask_register_bit(const uint8_t reg, const uint8_t bit);

    result_t write_pcd_command(const mfrc522::Command cmd);
    result_t write_picc_command_short_frame(const mfrc522::ISO14443Command cmd,
                                            uint8_t *ATQA, uint8_t &len);

    result_t anti_collision(const uint8_t clv, uint8_t *res, uint8_t &rlen,
                            const uint8_t collPos = 0);
    result_t select(const uint8_t clv, const uint8_t *uid, const uint8_t len,
                    uint8_t *res, uint8_t &rlen);

    result_t picc_authenticate(const mfrc522::ISO14443Command cmd,
                               const mfrc522::UID &uid,
                               const mfrc522::MifareKey &key,
                               const uint8_t block);

    bool exists_RATS(bool &available);

    result_t dump_mifare_classic_sector(const mfrc522::UID &uid,
                                        const mfrc522::MifareKey &key,
                                        const uint8_t sector);
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

constexpr uint8_t STATUS2_REG{0x08};
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
