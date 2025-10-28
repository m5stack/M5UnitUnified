#if 0





    m5::hal::error::error_t readWithTransaction(uint8_t* data, const size_t len);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister(const Reg reg, uint8_t* rbuf, const size_t len, const uint32_t delayMillis,
                      const bool stop = true);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool readRegister8(const Reg reg, uint8_t& result, const uint32_t delayMillis, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister16BE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register16E(reg, result, delayMillis, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister16LE(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register16E(reg, result, delayMillis, stop, false);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister32BE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register32E(reg, result, delayMillis, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool readRegister32LE(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop = true)
    {
        return read_register32E(reg, result, delayMillis, stop, false);
    }

    m5::hal::error::error_t writeWithTransaction(const uint8_t* data, const size_t len, const uint32_t exparam = 1);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    m5::hal::error::error_t writeWithTransaction(const Reg reg, const uint8_t* data, const size_t len,
                                                 const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister(const Reg reg, const uint8_t* buf = nullptr, const size_t len = 0U, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool writeRegister8(const Reg reg, const uint8_t value, const bool stop = true);

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister16BE(const Reg reg, const uint16_t value, const bool stop = true)
    {
        return write_register16E(reg, value, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister16LE(const Reg reg, const uint16_t value, const bool stop = true)
    {
        return write_register16E(reg, value, stop, false);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister32BE(const Reg reg, const uint32_t value, const bool stop = true)
    {
        return write_register32E(reg, value, stop, true);
    }

    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    inline bool writeRegister32LE(const Reg reg, const uint32_t value, const bool stop = true)
    {
        return write_register32E(reg, value, stop, false);
    }


    // I2C
    bool changeAddress(const uint8_t addr);  // Functions for dynamically addressable devices

template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool read_register16E(const Reg reg, uint16_t& result, const uint32_t delayMillis, const bool stop,
                          const bool endian);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool write_register16E(const Reg reg, const uint16_t value, const bool stop, const bool endifan);

template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool read_register32E(const Reg reg, uint32_t& result, const uint32_t delayMillis, const bool stop,
                          const bool endian);
    template <typename Reg,
              typename std::enable_if<std::is_integral<Reg>::value && std::is_unsigned<Reg>::value && sizeof(Reg) <= 2,
                                      std::nullptr_t>::type = nullptr>
    bool write_register32E(const Reg reg, const uint32_t value, const bool stop, const bool endifan);

#endif
