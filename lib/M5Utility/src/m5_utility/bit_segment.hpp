/*!
  @file bit_segment.hpp
  @brief A class for separating the bits of an integer variable and giving
  meaning to each

  @copyright M5Stack. All rights reserved.
  @license Licensed under the MIT license. See LICENSE file in the project root
  for full license information.
*/
#ifndef M5_UTILITY_BIT_SEGMENT_HPP
#define M5_UTILITY_BIT_SEGMENT_HPP

#include <cstddef>
#include <type_traits>

namespace m5 {
namespace utility {

/*!
  @class BitSegment
  @tparam LowerBits How many lower bits are used as a lower segment.
  @tparam T Integer type used as base class
  @note If the base class is signed, the upper bits excluding the sign bit are
  used
*/
template <size_t LowerBits, typename T>
class BitSegment {
   public:
    ///@cond
    using base_type = typename std::remove_const<
        typename std::remove_reference<T>::type>::type;
    constexpr static bool SIGNED = std::is_signed<base_type>::value;
    static_assert(std::is_integral<base_type>::value,
                  "Base type must be integral");
    static_assert(LowerBits > 0, "LowerBits must be not zero");
    static_assert(LowerBits <= (sizeof(base_type) * 8 - (SIGNED ? 1 : 0)),
                  "LowerBits too large");
    using unsigned_type = typename std::make_unsigned<base_type>::type;
    constexpr static unsigned_type UPPER_BITS =
        sizeof(unsigned_type) * 8U - LowerBits - (SIGNED ? 1 : 0);
    constexpr static unsigned_type LOWER_BITS =
        static_cast<unsigned_type>(LowerBits);
    constexpr static unsigned_type UPPER_SHIFT = LOWER_BITS;
    constexpr static unsigned_type UPPER_MASK =
        ((unsigned_type)1 << UPPER_BITS) - 1;
    constexpr static unsigned_type LOWER_MASK =
        ((unsigned_type)1 << LOWER_BITS) - 1;
    ///@endcond

    ///@name Constructor
    ///@{
    inline constexpr BitSegment() = default;  //!< @brief default
    //! @brief Copy
    inline constexpr BitSegment(const BitSegment& o) : _v(o._v) {
    }
    //! @brief Implicit conversion
    inline constexpr BitSegment(const base_type v) : _v(v) {
    }
    ///@}

    ///@name Assignment
    ///@{
    BitSegment& operator=(const BitSegment& o) {
        _v = o._v;
        return *this;
    }
    BitSegment& operator=(const base_type v) {
        _v = v;
        return *this;
    }
    ///@}

    ///@name Cast
    ///@{
    /*! @brief Cast to boolean */
    inline constexpr explicit operator bool() const {
        return _v;
    }
    //! @brief Cast to base_type (Implicit conversion)
    inline constexpr operator base_type() const {
        return _v;
    }
    ///@}

    ///@name Getter
    ///@{
    /*! @brief Gets the value of upper segment */
    inline constexpr unsigned_type upper() const {
        return (_v >> UPPER_SHIFT) & UPPER_MASK;
    }
    //! @brief Gets the value of lower segment
    inline constexpr unsigned_type lower() const {
        return _v & LOWER_MASK;
    }
    //! @brief Gets the raw value
    inline constexpr base_type raw() const {
        return _v;
    }
    ///@}

    ///@name Setter
    ///@{
    /*! @brief Set the value of upper segment */
    inline void upper(const unsigned_type v) {
        _v = (_v & ~(UPPER_MASK << UPPER_SHIFT)) |
             ((v & UPPER_MASK) << UPPER_SHIFT);
    }
    //! @brief Set the value of lower segment
    inline void lower(const unsigned_type v) {
        _v = (_v & ~LOWER_MASK) | (v & LOWER_MASK);
    }
    //! @brief Set the raw value
    inline void raw(const base_type v) {
        _v = v;
    }
    ///@}

   private:
    base_type _v{};
};

///@name Compare
///@brief Comparison by raw values
/// @related m5::utility::BitSegment
///@{
// Compare between same types.
template <size_t LowerBits, typename T>
bool operator==(const BitSegment<LowerBits, T>& a,
                const BitSegment<LowerBits, T>& b) {
    return a.raw() == b.raw();
}
template <size_t LowerBits, typename T>
bool operator!=(const BitSegment<LowerBits, T>& a,
                const BitSegment<LowerBits, T>& b) {
    return !(a == b);
}
template <size_t LowerBits, typename T>
bool operator<(const BitSegment<LowerBits, T>& a,
               const BitSegment<LowerBits, T>& b) {
    return a.raw() < b.raw();
}
template <size_t LowerBits, typename T>
bool operator>(const BitSegment<LowerBits, T>& a,
               const BitSegment<LowerBits, T>& b) {
    return b < a;
}
template <size_t LowerBits, typename T>
bool operator<=(const BitSegment<LowerBits, T>& a,
                const BitSegment<LowerBits, T>& b) {
    return !(a > b);
}
template <size_t LowerBits, typename T>
bool operator>=(const BitSegment<LowerBits, T>& a,
                const BitSegment<LowerBits, T>& b) {
    return !(a < b);
}
// Compare between BitSegment and integer
template <size_t LowerBits, typename T>
bool operator==(const BitSegment<LowerBits, T>& a, const int b) {
    return a.raw() == b;
}
template <size_t LowerBits, typename T>
bool operator!=(const BitSegment<LowerBits, T>& a, const int b) {
    return !(a == b);
}
template <size_t LowerBits, typename T>
bool operator<(const BitSegment<LowerBits, T>& a, const int b) {
    return a.raw() < b;
}
template <size_t LowerBits, typename T>
bool operator>(const BitSegment<LowerBits, T>& a, const int b) {
    return b < a;
}
template <size_t LowerBits, typename T>
bool operator<=(const BitSegment<LowerBits, T>& a, const int b) {
    return !(a > b);
}
template <size_t LowerBits, typename T>
bool operator>=(const BitSegment<LowerBits, T>& a, const int b) {
    return !(a < b);
}
// Compare between integer and BitSegment
template <size_t LowerBits, typename T>
bool operator==(const int a, const BitSegment<LowerBits, T>& b) {
    return a == b.raw();
}
template <size_t LowerBits, typename T>
bool operator!=(const int a, const BitSegment<LowerBits, T>& b) {
    return !(a == b);
}
template <size_t LowerBits, typename T>
bool operator<(const int a, const BitSegment<LowerBits, T>& b) {
    return a < b.raw();
}
template <size_t LowerBits, typename T>
bool operator>(const int a, const BitSegment<LowerBits, T>& b) {
    return b < a;
}

template <size_t LowerBits, typename T>
bool operator<=(const int a, const BitSegment<LowerBits, T>& b) {
    return !(a > b);
}
template <size_t LowerBits, typename T>
bool operator>=(const int a, const BitSegment<LowerBits, T>& b) {
    return !(a < b);
}
///@}

}  // namespace utility
}  // namespace m5
#endif
