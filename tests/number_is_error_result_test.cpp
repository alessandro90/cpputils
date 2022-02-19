#include "cpputils/types/number.hpp"
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <limits>

using namespace cpputils;
using namespace cpputils::detail;
using namespace Catch::literals;

namespace {
template <typename T>
consteval auto max_val() {
    return std::numeric_limits<T>::max();
}

template <typename T>
consteval auto min_val() {
    return std::numeric_limits<T>::min();
}
}  // namespace


TEMPLATE_TEST_CASE("Check unsigned addition error",  // NOLINT
                   "",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t) {
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(is_error_result_add(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_add(lhs, rhs));
    }
}

TEMPLATE_TEST_CASE("Check unsigned subtraction error",  // NOLINT
                   "",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t) {
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(is_error_result_sub(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_sub(lhs, rhs));
    }
}


TEMPLATE_TEST_CASE("Check unsigned multiplication error",  // NOLINT
                   "",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t) {
    {
        TestType const lhs{max_val<TestType>() / 2 + 1};
        TestType const rhs{2};
        REQUIRE(is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>() / 2};
        TestType const rhs{2};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
}


TEMPLATE_TEST_CASE("Check unsigned division error",  // NOLINT
                   "",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t) {
    {
        TestType const lhs{1};
        TestType const rhs{0};
        REQUIRE(is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{0};
        REQUIRE(is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{1};
        TestType const rhs{1};
        REQUIRE(!is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(!is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{max_val<TestType>()};
        REQUIRE(!is_error_result_div(lhs, rhs));
    }
}

TEMPLATE_TEST_CASE("Check unsigned and signed modulo error",  // NOLINT
                   "",
                   std::uint8_t,
                   std::uint16_t,
                   std::uint32_t,
                   std::uint64_t,
                   std::size_t,
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   std::ptrdiff_t) {
    {
        TestType const lhs{0};
        TestType const rhs{0};
        REQUIRE(is_error_result_mod(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(is_error_result_mod(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(is_error_result_mod(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{max_val<TestType>()};
        REQUIRE(!is_error_result_mod(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{min_val<TestType>()};
        if (std::is_signed_v<TestType>) {
            REQUIRE(!is_error_result_mod(lhs, rhs));
        } else {
            REQUIRE(is_error_result_mod(lhs, rhs));
        }
    }
}

TEMPLATE_TEST_CASE("Check signed addition error",  // NOLINT
                   "",
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   std::ptrdiff_t) {
    // Positive numbers
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(is_error_result_add(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_add(lhs, rhs));
    }
    // Negative numbers
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(is_error_result_add(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_add(lhs, rhs));
    }
    // Mixed numbers
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(!is_error_result_add(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(!is_error_result_add(lhs, rhs));
    }
}


TEMPLATE_TEST_CASE("Check signed subtraction error",  // NOLINT
                   "",
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   std::ptrdiff_t) {
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(!is_error_result_sub(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(is_error_result_sub(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(is_error_result_sub(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(!is_error_result_sub(lhs, rhs));
    }
}

TEMPLATE_TEST_CASE("Check signed multiplication error",  // NOLINT
                   "",
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   std::ptrdiff_t) {
    {
        TestType const lhs{0};
        TestType const rhs{0};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>() / 2 + 1};
        TestType const rhs{2};
        REQUIRE(is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{0};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{max_val<TestType>()};
        TestType const rhs{1};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    ///
    {
        TestType const lhs{-1};
        TestType const rhs{min_val<TestType>()};
        REQUIRE(is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{2};
        TestType const rhs{max_val<TestType>() / 2 + 1};
        REQUIRE(is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{max_val<TestType>()};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
    {
        TestType const lhs{1};
        TestType const rhs{max_val<TestType>()};
        REQUIRE(!is_error_result_mul(lhs, rhs));
    }
}

TEMPLATE_TEST_CASE("Check signed division error",  // NOLINT
                   "",
                   std::int8_t,
                   std::int16_t,
                   std::int32_t,
                   std::int64_t,
                   std::ptrdiff_t) {
    {
        TestType const lhs{0};
        TestType const rhs{0};
        REQUIRE(is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{0};
        TestType const rhs{1};
        REQUIRE(!is_error_result_div(lhs, rhs));
    }
    {
        TestType const lhs{min_val<TestType>()};
        TestType const rhs{-1};
        REQUIRE(is_error_result_div(lhs, rhs));
    }
}