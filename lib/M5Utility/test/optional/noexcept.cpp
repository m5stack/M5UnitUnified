#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

TEST(Optional, Noexcept) {
  m5::stl::optional<int> o1{4};
  m5::stl::optional<int> o2{42};
  {
    SCOPED_TRACE("comparison with nullopt");
    EXPECT_TRUE(noexcept(o1 == m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt == o1));
    EXPECT_TRUE(noexcept(o1 != m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt != o1));
    EXPECT_TRUE(noexcept(o1 < m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt < o1));
    EXPECT_TRUE(noexcept(o1 <= m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt <= o1));
    EXPECT_TRUE(noexcept(o1 > m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt > o1));
    EXPECT_TRUE(noexcept(o1 >= m5::stl::nullopt));
    EXPECT_TRUE(noexcept(m5::stl::nullopt >= o1));
  }
  {
    SCOPED_TRACE("swap");
      //TODO see why this fails
#if !defined(_MSC_VER) || _MSC_VER > 1900
    EXPECT_TRUE(noexcept(swap(o1, o2)) == noexcept(o1.swap(o2)));

    struct nothrow_swappable {
      nothrow_swappable &swap(const nothrow_swappable &) noexcept {
        return *this;
      }
    };

    struct throw_swappable {
      throw_swappable() = default;
      throw_swappable(const throw_swappable &) {}
      throw_swappable(throw_swappable &&) {}
      throw_swappable &swap(const throw_swappable &) { return *this; }
    };

    m5::stl::optional<nothrow_swappable> ont;
    m5::stl::optional<throw_swappable> ot;

    EXPECT_TRUE(noexcept(ont.swap(ont)));
    EXPECT_TRUE(!noexcept(ot.swap(ot)));
    #endif
  }
  {
    SCOPED_TRACE("constructors");
      //TODO see why this fails
#if !defined(_MSC_VER) || _MSC_VER > 1900
    EXPECT_TRUE(noexcept(m5::stl::optional<int>{}));
    EXPECT_TRUE(noexcept(m5::stl::optional<int>{m5::stl::nullopt}));

    struct nothrow_move {
      nothrow_move(nothrow_move &&) noexcept = default;
    };

    struct throw_move {
      throw_move(throw_move &&){};
    };

    using nothrow_opt = m5::stl::optional<nothrow_move>;
    using throw_opt = m5::stl::optional<throw_move>;

    EXPECT_TRUE(std::is_nothrow_move_constructible<nothrow_opt>::value);
    EXPECT_TRUE(!std::is_nothrow_move_constructible<throw_opt>::value);
#endif
  }
  {
    SCOPED_TRACE("assignment");
    EXPECT_TRUE(noexcept(o1 = m5::stl::nullopt));

    struct nothrow_move_assign {
      nothrow_move_assign() = default;
      nothrow_move_assign(nothrow_move_assign &&) noexcept = default;
      nothrow_move_assign &operator=(const nothrow_move_assign &) = default;
    };

    struct throw_move_assign {
      throw_move_assign() = default;
      throw_move_assign(throw_move_assign &&){};
      throw_move_assign &operator=(const throw_move_assign &) { return *this; }
    };

    using nothrow_opt = m5::stl::optional<nothrow_move_assign>;
    using throw_opt = m5::stl::optional<throw_move_assign>;

    EXPECT_TRUE(
        noexcept(std::declval<nothrow_opt>() = std::declval<nothrow_opt>()));
    EXPECT_TRUE(!noexcept(std::declval<throw_opt>() = std::declval<throw_opt>()));
  }
  {
    SCOPED_TRACE("observers");
    EXPECT_TRUE(noexcept(static_cast<bool>(o1)));
    EXPECT_TRUE(noexcept(o1.has_value()));
  }
  {
    SCOPED_TRACE("modifiers");
    EXPECT_TRUE(noexcept(o1.reset()));
  }
}
