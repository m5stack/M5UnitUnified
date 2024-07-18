/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file circular_buffer.hpp
  @brief Circular buffer with STL-like interface
*/
#ifndef M5_UTILITY_CONTAINER_CIRCULAR_BUFFER_HPP
#define M5_UTILITY_CONTAINER_CIRCULAR_BUFFER_HPP

#include <cstddef>
#include <array>
#include <cassert>
#include "../stl/optional.hpp"

namespace m5 {
namespace container {

/// @cond
template <class CB>
class CircularBufferIterator {
   public:
    using value_type      = typename CB::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer         = typename CB::value_type*;
    using reference       = typename CB::reference;
    //    using iterator_category = std::random_access_iterator_tag;
    using iterator_category = std::bidirectional_iterator_tag;

    CircularBufferIterator(CB& cb, std::size_t tail) : _cb(&cb), _idx(tail) {
    }
    CircularBufferIterator(const CircularBufferIterator& o) = default;
    CircularBufferIterator& operator=(const CircularBufferIterator& o) =
        default;

    inline typename CB::const_reference operator*() {
        return this->_cb->_buf[_idx % _cb->capacity()];
    }

    inline pointer operator->() {
        return &(this->_cb->_buf[_idx % _cb->capacity()]);
    }

    inline CircularBufferIterator& operator++() {
        ++_idx;
        return *this;
    }
    inline CircularBufferIterator& operator--() {
        --_idx;
        return *this;
    }
    inline CircularBufferIterator operator++(int) {
        CircularBufferIterator it = *this;
        ++*this;
        return it;
    }
    inline CircularBufferIterator operator--(int) {
        CircularBufferIterator it = *this;
        --*this;
        return it;
    }

    inline CircularBufferIterator& operator+=(std::size_t n) {
        _idx += n;
        return *this;
    }
    inline CircularBufferIterator& operator-=(std::size_t n) {
        _idx -= n;
        return *this;
    }
    inline CircularBufferIterator operator+(std::size_t n) const {
        return CircularBufferIterator(*this) += n;
    }
    inline CircularBufferIterator operator-(std::size_t n) const {
        return CircularBufferIterator(*this) -= n;
    }
    inline std::size_t operator-(const CircularBufferIterator& o) {
        assert(_cb == o._cb && "Diffrent iterators of container");
        assert(_idx >= o._idx && "o must be lesser than this.");
        return _idx - o._idx;
    }

    inline bool operator==(const CircularBufferIterator& b) const {
        return (_cb == b._cb) && _idx == b._idx;
    }
    inline bool operator<(const CircularBufferIterator& b) const {
        return _idx < b._idx;
    }
    inline bool operator!=(const CircularBufferIterator& b) const {
        return !(*this == b);
    }
    inline bool operator>(const CircularBufferIterator& b) const {
        return b < (*this);
    }
    inline bool operator<=(const CircularBufferIterator& b) const {
        return !(*this > b);
    }
    inline bool operator>=(const CircularBufferIterator& b) const {
        return !(*this < b);
    }

   private:
    CB* _cb{};
    size_t _idx{};
};
/// @endcond

/*!
  @class CircularBuffer
  @brief Type CircularBuffer  giving size in constructor
  @tparam T Type of the element
 */
template <typename T>
class CircularBuffer {
   public:
    using value_type      = T;
    using size_type       = size_t;
    using reference       = T&;
    using const_reference = const T&;
    using return_type     = m5::stl::optional<value_type>;
    friend class CircularBufferIterator<CircularBuffer>;
    using const_iterator = CircularBufferIterator<CircularBuffer>;

    ///@name Constructor
    ///@{
    CircularBuffer() = delete;
    explicit CircularBuffer(const size_t n) {
        assert(n != 0 && "Illegal size");
        _cap = n;
        _buf.resize(n);
    }
    CircularBuffer(const size_type n, const_reference value)
        : CircularBuffer(n) {
        assign(n, value);
    }
    template <class InputIter>
    CircularBuffer(const size_type n, InputIter first, InputIter last)
        : CircularBuffer(n) {
        assign(first, last);
    }
    CircularBuffer(const size_type n, std::initializer_list<T> il)
        : CircularBuffer(n, il.begin(), il.end()) {
    }

    CircularBuffer(const CircularBuffer&) = default;
    CircularBuffer(CircularBuffer&&)      = default;
    ///@}

    /// @name Assignment
    /// @{
    /*! @brief Copy */
    CircularBuffer& operator=(const CircularBuffer&) = default;
    //! @brief Move
    CircularBuffer& operator=(CircularBuffer&&) = default;
    /*!
      @brief Replaces the contents with copies of those in the range [first,
      last)
      @param first,last The Range to copy the elements from
     */
    template <class InputIterator>
    void assign(InputIterator first, InputIterator last) {
        clear();
        size_type sz = last - first;
        if (sz > _cap) {
            first += (sz - _cap);
        }
        auto n = std::min(_cap, sz);
        while (n--) {
            push_back(*first++);
        }
    }
    /*!
      @brief assigns values to the container
      @param n Number of elements
      @param v Value to assign to the elements
      @note Fill with the value as many times as n or the capacity
     */
    void assign(size_type n, const_reference v) {
        clear();
        n = std::min(_cap, n);
        while (n--) {
            push_back(v);
        }
    }
    /*!
      @brief assigns values to the container
      @param il Initializer list from which the copy is made
     */
    inline void assign(std::initializer_list<T> il) {
        assign(il.begin(), il.end());
    }
    /// @}

    ///@name Element access
    ///@{
    /*!
      @brief Access the first element
      @return m5::stl::optional<value_type>
    */
    inline return_type front() const {
        return !empty() ? m5::stl::make_optional(_buf[_tail])
                        : m5::stl::nullopt;
    }
    /*!
      @brief Access the last element
      @return m5::stl::optional<value_type>
    */
    inline return_type back() const {
        return !empty()
                   ? m5::stl::make_optional(_buf[(_head - 1 + _cap) % _cap])
                   : m5::stl::nullopt;
    }
    /*!
      @brief Access specified element
      @return Reference to the requested element
    */
    inline const_reference operator[](size_type i) const& {
        assert(size() > 0 && "container empty");
        assert(i < size() && "index overflow");
        return _buf[(_tail + i) % _cap];
    }
    /*!
      @brief Access specified element with bounds checking
      @return m5::stl::optional<value_type>
    */
    inline return_type at(size_type i) const {
        return (!empty() && i < size())
                   ? m5::stl::make_optional(_buf[(_tail + i) % _cap])
                   : m5::stl::nullopt;
    }
    /*!
      @brief Read from buffer
      @param[out] outbuf Output buffer
      @param num Max elements of output buffer
      @return Number of elements read
     */
    size_t read(value_type* outbuf, const size_t num) {
        size_t sz = std::min(num, size());
        if (sz == 0) {
            return sz;
        }
        auto tail   = _tail;
        auto src    = &_buf[tail];
        size_t elms = std::min(_cap - tail, sz);

        std::copy(src, src + elms, outbuf);
        tail       = (tail + elms) % _cap;
        size_t ret = elms;

        if (elms < sz) {
            outbuf += elms;
            src  = &_buf[tail];
            elms = sz - elms;

            std::copy(src, src + elms, outbuf);
            ret += elms;
        }
        return ret;
    }
    /// @}

    /// @name Iterators
    /// @brief This class has const_iterator only.
    /// @{
    const_iterator begin() noexcept {
        return const_iterator(*this, _tail);
    }
    const_iterator end() noexcept {
        return const_iterator(*this, _tail + size());
    }
    const_iterator cbegin() noexcept {
        return const_iterator(*this, _tail);
    }
    const_iterator cend() noexcept {
        return const_iterator(*this, _tail + size());
    }
    /// @}

    ///@name Capacity
    ///@{
    /*!
      @brief checks whether the container is empty
      @return True if empty
    */
    inline bool empty() const {
        return !full() && (_head == _tail);
    }
    /*!
      @brief checks whether the container is full
      @return True if full
    */
    inline bool full() const {
        return _full;
    }
    /*!
      @brief returns the number of elements
    */
    inline size_type size() const {
        return full() ? _cap
                      : (_head >= _tail ? _head - _tail : _cap + _head - _tail);
    }
    /*!
      @brief Returns the number of elements that can be held in currently
      storage
     */
    inline size_type capacity() const {
        return _cap;
    }
    ///@}

    ///@name Modifiers
    ///@{
    /*! @brief Clears the contents */
    void clear() {
        _full = false;
        _head = _tail = 0U;
    }
    //!  @brief Adds an element to the top
    void push_front(const value_type& v) {
        _tail       = (_tail - 1 + _cap) % _cap;
        _buf[_tail] = v;
        if (_full) {
            _head = (_head - 1 + _cap) % _cap;
        }
        _full = (_head == _tail);
    }
    //! @brief Adds an element to the end
    void push_back(const value_type& v) {
        _buf[_head] = v;
        _head       = (_head + 1) % _cap;
        if (_full) {
            _tail = (_tail + 1) % _cap;
        }
        _full = (_head == _tail);
    }
    //! @brief removes the top element
    inline void pop_front() {
        if (!empty()) {
            _tail = (_tail + 1) % _cap;
            _full = false;
        }
    }
    //! @brief removes the end element
    inline void pop_back() {
        if (!empty()) {
            _head = (_head - 1 + _cap) % _cap;
            _full = false;
        }
    }
    ///@}

    ///@name Operations
    ///@{
    /*!
      @brief Assigns the value to all elements in the container
      @param v Value to assign to the elements
     */
    void fill(const value_type& v) {
        clear();
        std::fill(_buf.begin(), _buf.end(), v);
        _full = true;
    }
    /*!
      @brief Swaps the contents
      @param o Ccontainer to exchange the contents with
    */
    void swap(CircularBuffer& o) {
        if (this != &o) {
            std::swap(_buf, o._buf);
            std::swap(_cap, o._cap);
            std::swap(_head, o._head);
            std::swap(_tail, o._tail);
            std::swap(_full, o._full);
        }
    }
    ///@}

    //   private:
    std::vector<T> _buf{};
    size_t _cap{}, _head{}, _tail{};
    bool _full{};
};

/*!
  @class FixedCircularBuffer
  @brief Type CircularBuffer  giving size in template parameter
  @tparam T Type of the element
  @tpatam N Capacity of the buffer
*/
template <typename T, size_t N>
class FixedCircularBuffer : public CircularBuffer<T> {
   public:
    using value_type      = T;
    using size_type       = size_t;
    using reference       = T&;
    using const_reference = const T&;
    using return_type     = m5::stl::optional<value_type>;
    friend class CircularBufferIterator<CircularBuffer<T>>;
    using const_iterator = CircularBufferIterator<CircularBuffer<T>>;

    FixedCircularBuffer() : CircularBuffer<T>(N) {
    }
    FixedCircularBuffer(const size_type n, const_reference value)
        : CircularBuffer<T>(N) {
        CircularBuffer<T>::assign(n, value);
    }
    template <class InputIter>
    FixedCircularBuffer(InputIter first, InputIter last)
        : CircularBuffer<T>(N, first, last) {
    }
    FixedCircularBuffer(std::initializer_list<T> il)
        : CircularBuffer<T>(N, il) {
    }

    FixedCircularBuffer(const FixedCircularBuffer&) = default;
    FixedCircularBuffer(FixedCircularBuffer&&)      = default;

    FixedCircularBuffer& operator=(const FixedCircularBuffer&) = default;
    FixedCircularBuffer& operator=(FixedCircularBuffer&&)      = default;
};

}  // namespace container
}  // namespace m5

namespace std {
/*!
  @brief Specializes the std::swap algorithm
  @related m5::container::CircularBuffer
  @param a,b Containers whose contents to swap
*/
template <typename T>
inline void swap(m5::container::CircularBuffer<T>& a,
                 m5::container::CircularBuffer<T>& b) {
    a.swap(b);
}
}  // namespace std

#endif
