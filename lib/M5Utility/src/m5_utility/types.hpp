/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file types.hpp
  @brief Type and enumerator definitions
*/
#ifndef M5_UTILITY_TYPES_HPP
#define M5_UTILITY_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <utility>
#include <tuple>
#include "stl/endianness.hpp"

namespace m5 {
namespace types {

/*!
  @struct U16
  @tparam DELittle Endian type specification<br> true: Little false: Big
  @brief Endian-compliant uint16
*/
template <bool DELittle>
union U16 {
    /// @name Constrcutor
    ///@{
    /*! @brief default constructor */
    constexpr U16() : u16{0} {
    }
    //!@brief from uint16_t
#if 0
    template <bool PELittle = m5::endian::little>
    constexpr explicit U16(const uint16_t v) : u8{
        static_cast<uint8_t>(
            DELittle == PELittle ? (v & 0XFF) : (v >> 8)),
        u8[1] = static_cast<uint8_t>(
            DELittle == PELittle ? (v >> 8) : (v & 0xFF)) } {}
#else
    template <bool PELittle = m5::endian::little>
    explicit U16(const uint16_t v) {
        set<PELittle>(v);
    }

#endif
    /*!
      @brief from uint8_t pointer
      @warning The area pointed to by the pointer must be at least 2 bytes
     */
    constexpr explicit U16(const uint8_t* p) : u8{*p, *(p + 1)} {
    }
    //! @brief Stored in order of high and low
    constexpr U16(const uint8_t high, const uint8_t low) : u8{high, low} {
    }
    constexpr U16(const U16&) = default;  //!< @brief as default
    constexpr U16(U16&& o)    = default;  //!< @brief as default
    ///@}

    ///@name Assignment
    ///@{
    U16& operator=(const U16&) = default;
    U16& operator=(U16&&)      = default;
    template <bool PELittle = m5::endian::little>
    U16& operator=(const uint16_t v) {
        set<PELittle>(v);
        return *this;
    }
    template <typename H, typename L>
    U16& operator=(const std::pair<H, L>& o) {
        static_assert(std::is_integral<H>::value && std::is_integral<L>::value,
                      "HIGH & LOW Must be integral");
        u8[0] = static_cast<uint8_t>(o.first);
        u8[1] = static_cast<uint8_t>(o.second);
        return *this;
    }
    ///@}

    ///@name Cast
    ///@{
    /*! @brief To boolean */
    inline explicit operator bool() const {
        return u16;
    }
    /*! @brief To const uint8_t* */
    inline explicit operator const uint8_t*() const {
        return data();
    }
    /*! @brief To uint8_t* */
    inline explicit operator uint8_t*() const {
        return data();
    }
    //! @brief To uint16_t on processor endianness
    inline explicit operator uint16_t() const {
        return get();
    }
    ///@}

    /*!
      @brief Set value with specified endianness
      @tparam PELittle Endianness (default as processor endianness)
    */
    template <bool PELittle = m5::endian::little>
    inline void set(const uint16_t v) {
        if (DELittle == PELittle) {
            u16 = v;
        } else {
            u8[0] = static_cast<uint8_t>(v >> 8);
            u8[1] = static_cast<uint8_t>(v & 0xFF);
        }
    }
    /*!
      @brief Gets value with specified endianness
      @tparam PELittle Endianness (default as processor endianness)
     */
    template <bool PELittle = m5::endian::little>
    inline uint16_t get() const {
        uint16_t r{u16};
        if (DELittle != PELittle) {
            r = U16<DELittle>{u8[1], u8[0]}.u16;
        }
        return r;
    };
    //! @brief Gets the high byte
    inline uint8_t high() const {
        return u8[0];
    }
    //! @brief Gets the low byte
    inline uint8_t low() const {
        return u8[1];
    }
    //! @brief Gets the const pointer
    inline const uint8_t* data() const {
        return u8;
    }
    //! @brief Gets the pointer
    inline uint8_t* data() {
        return u8;
    }
    //! @brief Gets size in uint8_t units.
    inline size_t size() const {
        return 2;
    }

    uint16_t u16{};  //!< @brief Raw value
    uint8_t u8[2];   //!< @brief Raw value according to uint8_t
};

using big_uint16_t    = U16<false>;
using little_uint16_t = U16<true>;

///@name Compare
/// @related m5::types::U16
///@{
// ==
inline bool operator==(const big_uint16_t& a, const big_uint16_t& b) {
    return a.u16 == b.u16;
}
inline bool operator==(const big_uint16_t& a, const little_uint16_t& b) {
    return std::tie(a.u8[0], a.u8[1]) == std::tie(b.u8[1], b.u8[0]);
}
inline bool operator==(const little_uint16_t& a, const big_uint16_t& b) {
    return std::tie(a.u8[1], a.u8[0]) == std::tie(b.u8[0], b.u8[1]);
}
inline bool operator==(const little_uint16_t& a, const little_uint16_t& b) {
    return a.u16 == b.u16;
}
// !=
inline bool operator!=(const big_uint16_t& a, const big_uint16_t& b) {
    return !(a == b);
}
inline bool operator!=(const big_uint16_t& a, const little_uint16_t& b) {
    return !(a == b);
}
inline bool operator!=(const little_uint16_t& a, const big_uint16_t& b) {
    return !(a == b);
}
inline bool operator!=(const little_uint16_t& a, const little_uint16_t& b) {
    return !(a == b);
}
// <
inline bool operator<(const big_uint16_t& a, const big_uint16_t& b) {
    return a.u16 < b.u16;
}
inline bool operator<(const big_uint16_t& a, const little_uint16_t& b) {
    return std::tie(a.u8[0], a.u8[1]) < std::tie(b.u8[1], b.u8[0]);
}
inline bool operator<(const little_uint16_t& a, const big_uint16_t& b) {
    return std::tie(a.u8[1], a.u8[0]) < std::tie(b.u8[0], b.u8[1]);
}
inline bool operator<(const little_uint16_t& a, const little_uint16_t& b) {
    return a.u16 < b.u16;
}
// >
inline bool operator>(const big_uint16_t& a, const big_uint16_t& b) {
    return b < a;
}
inline bool operator>(const big_uint16_t& a, const little_uint16_t& b) {
    return b < a;
}
inline bool operator>(const little_uint16_t& a, const big_uint16_t& b) {
    return b < a;
}
inline bool operator>(const little_uint16_t& a, const little_uint16_t& b) {
    return b < a;
}
// <=
inline bool operator<=(const big_uint16_t& a, const big_uint16_t& b) {
    return !(a > b);
}
inline bool operator<=(const big_uint16_t& a, const little_uint16_t& b) {
    return !(a > b);
}
inline bool operator<=(const little_uint16_t& a, const big_uint16_t& b) {
    return !(a > b);
}
inline bool operator<=(const little_uint16_t& a, const little_uint16_t& b) {
    return !(a > b);
}
// >=
inline bool operator>=(const big_uint16_t& a, const big_uint16_t& b) {
    return !(a < b);
}
inline bool operator>=(const big_uint16_t& a, const little_uint16_t& b) {
    return !(a < b);
}
inline bool operator>=(const little_uint16_t& a, const big_uint16_t& b) {
    return !(a < b);
}
inline bool operator>=(const little_uint16_t& a, const little_uint16_t& b) {
    return !(a < b);
}
///@}

}  // namespace types
}  // namespace m5
#endif
