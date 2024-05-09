#include <gtest/gtest.h>
#include <m5_utility/stl/expected.hpp>
#include <string>

// Old versions of GCC don't have the correct trait names. Could fix them up if needs be.
#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ <= 9 &&              \
     !defined(__clang__))
// nothing for now
#else
TEST(Expected, Triviality) {
    EXPECT_TRUE((std::is_trivially_copy_constructible<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_trivially_copy_assignable<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_trivially_move_constructible<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_trivially_move_assignable<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_trivially_destructible<m5::stl::expected<int,int>>::value));

    EXPECT_TRUE((std::is_trivially_copy_constructible<m5::stl::expected<void,int>>::value));
    EXPECT_TRUE((std::is_trivially_move_constructible<m5::stl::expected<void,int>>::value));
    EXPECT_TRUE((std::is_trivially_destructible<m5::stl::expected<void,int>>::value));


    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        EXPECT_TRUE((std::is_trivially_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_trivially_copy_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_trivially_move_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_trivially_move_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_trivially_destructible<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&){}
            T(T&&) {}
            T& operator=(const T&) { return *this; }
            T& operator=(T&&) { return *this; }
            ~T(){}
        };
        EXPECT_TRUE(!(std::is_trivially_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_trivially_copy_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_trivially_move_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_trivially_move_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_trivially_destructible<m5::stl::expected<T,int>>::value));
    }

}

TEST(Expected, Deletion) {
    EXPECT_TRUE((std::is_copy_constructible<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_copy_assignable<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_move_constructible<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_move_assignable<m5::stl::expected<int,int>>::value));
    EXPECT_TRUE((std::is_destructible<m5::stl::expected<int,int>>::value));

    {
        struct T {
            T()=default;
        };
        EXPECT_TRUE((std::is_default_constructible<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(int){}
        };
        EXPECT_TRUE(!(std::is_default_constructible<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        EXPECT_TRUE((std::is_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_copy_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_move_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_move_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_destructible<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=delete;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=delete;
        };
        EXPECT_TRUE(!(std::is_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_copy_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((!std::is_move_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_move_assignable<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=default;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=default;
        };
        EXPECT_TRUE(!(std::is_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE(!(std::is_copy_assignable<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_move_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_move_assignable<m5::stl::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=default;
            T(T&&)=delete;
            T& operator=(const T&)=default;
            T& operator=(T&&)=delete;
        };
        EXPECT_TRUE((std::is_copy_constructible<m5::stl::expected<T,int>>::value));
        EXPECT_TRUE((std::is_copy_assignable<m5::stl::expected<T,int>>::value));
    }

	{
		m5::stl::expected<int, int> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<int, std::string> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<std::string, int> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		m5::stl::expected<std::string, std::string> e;
		EXPECT_TRUE(std::is_default_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		EXPECT_TRUE(std::is_move_assignable<decltype(e)>::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(decltype(e))::value);
		EXPECT_TRUE(!TL_EXPECTED_IS_TRIVIALLY_COPY_ASSIGNABLE(decltype(e))::value);
#	if !defined(TL_EXPECTED_GCC49)
		EXPECT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		EXPECT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

}


#endif
