#include <gtest/gtest.h>
#include <m5_utility/stl/optional.hpp>

// Old versions of GCC don't have the correct trait names. Could fix them up if needs be.
#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
// nothing for now
#else
TEST(Optional,Triviality) {
    EXPECT_TRUE(std::is_trivially_copy_constructible<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_trivially_copy_assignable<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_trivially_move_constructible<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_trivially_move_assignable<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_trivially_destructible<m5::stl::optional<int>>::value);

    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        EXPECT_TRUE(std::is_trivially_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_trivially_copy_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_trivially_move_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_trivially_move_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_trivially_destructible<m5::stl::optional<T>>::value);
    }

    {
        struct T {
            T(const T&){}
            T(T&&) {};
            T& operator=(const T&) { return *this; }
            T& operator=(T&&) { return *this; };
            ~T(){}
        };
        EXPECT_TRUE(!std::is_trivially_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_trivially_copy_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_trivially_move_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_trivially_move_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_trivially_destructible<m5::stl::optional<T>>::value);
    }

}

TEST(Optional, Deletion) {
    EXPECT_TRUE(std::is_copy_constructible<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_copy_assignable<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_move_constructible<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_move_assignable<m5::stl::optional<int>>::value);
    EXPECT_TRUE(std::is_destructible<m5::stl::optional<int>>::value);

    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        EXPECT_TRUE(std::is_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_copy_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_move_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_move_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_destructible<m5::stl::optional<T>>::value);
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=delete;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=delete;
        };
        EXPECT_TRUE(!std::is_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_copy_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_move_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_move_assignable<m5::stl::optional<T>>::value);
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=default;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=default;
        };
        EXPECT_TRUE(!std::is_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(!std::is_copy_assignable<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_move_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_move_assignable<m5::stl::optional<T>>::value);
    }

    {
        struct T {
            T(const T&)=default;
            T(T&&)=delete;
            T& operator=(const T&)=default;
            T& operator=(T&&)=delete;
        };
        EXPECT_TRUE(std::is_copy_constructible<m5::stl::optional<T>>::value);
        EXPECT_TRUE(std::is_copy_assignable<m5::stl::optional<T>>::value);
    }
}
#endif
