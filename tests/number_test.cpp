#include "cpputils/types/number.hpp"
#include <catch2/catch_all.hpp>
#include <unordered_map>
// #include <iostream>

using namespace cpputils;
using namespace cpputils::literals;
using namespace Catch::literals;

namespace {
template <typename From, typename... To>
void check_conversion(std::int64_t value) {
    using inner_from_t = detail::inner_type_t<From>;
    REQUIRE(((From{static_cast<inner_from_t>(value)}.template as<To>()
              == To{static_cast<detail::inner_type_t<To>>(value)})
             && ...));
}
}  // namespace

TEMPLATE_TEST_CASE("Integral number objects",  // NOLINT
                   "",
                   i8,
                   u8,
                   i16,
                   u16,
                   i32,
                   u32,
                   i64,
                   u64,
                   isize,
                   usize) {
    using type = detail::inner_type_t<TestType>;

    SECTION("Numbers are default constructible") {
        TestType const x{};
        REQUIRE(x.val() == 0);  // NOLINT
    }
    SECTION("Numbers are constructible with an argument") {
        for (auto i : {0, 1, 2}) {
            auto const j = static_cast<type>(i);
            TestType const x{j};
            REQUIRE(x.val() == j);  // NOLINT
        }
    }
    SECTION("Numbers are copy-constructible") {
        {
            TestType const x{10};
            auto const y{x};
            REQUIRE(x.val() == y.val());  // NOLINT
        }
        {
            TestType const x{10};
            auto const y = x;
            REQUIRE(x.val() == y.val());  // NOLINT
        }
    }
    SECTION("Numbers are copy-assignable") {
        TestType const x{4};
        TestType y{};
        y = x;
        REQUIRE(x.val() == y.val());  // NOLINT
    }
    SECTION("Numbers are comparable") {
        {
            TestType const x{5};
            TestType const y{4};
            REQUIRE((x >= y));  // NOLINT
            REQUIRE((x != y));  // NOLINT

            REQUIRE_FALSE((x < y));  // NOLINT
            REQUIRE_FALSE((x <= y));  // NOLINT
            REQUIRE_FALSE((x == y));  // NOLINT
        }
        {
            TestType const x{9};
            TestType const y{x};
            REQUIRE((x == y));  // NOLINT
            REQUIRE_FALSE((x != y));  // NOLINT
        }
    }
    SECTION("Post- and pre-increment") {
        {
            TestType x{};
            ++x;
            REQUIRE((x == TestType{1}));  // NOLINT
        }
        {
            TestType x{1};
            --x;
            REQUIRE((x == TestType{0}));  // NOLINT
        }
        {
            TestType x{};
            auto y = x++;
            REQUIRE((x == TestType{1}));  // NOLINT
            REQUIRE((y == TestType{0}));  // NOLINT
        }
        {
            TestType x{1};
            auto y = x--;
            REQUIRE((x == TestType{0}));  // NOLINT
            REQUIRE((y == TestType{1}));  // NOLINT
        }
    }
    SECTION("In-place arithmetic") {
        {
            auto const start_value = 1;
            auto const delta = 10;
            TestType x{start_value};
            x += TestType{delta};
            REQUIRE((x == TestType{start_value + delta}));  // NOLINT
        }
        {
            auto const start_value = 10;
            auto const delta = 5;
            TestType x{start_value};
            x -= TestType{delta};
            REQUIRE((x == TestType{start_value - delta}));  // NOLINT
        }
        {
            auto const start_value = 2;
            auto const delta = 5;
            TestType x{start_value};
            x *= TestType{delta};
            REQUIRE((x == TestType{start_value * delta}));  // NOLINT
        }
        {
            auto const start_value = 100;
            auto const delta = 3;
            TestType x{start_value};
            x /= TestType{delta};
            REQUIRE((x == TestType{start_value / delta}));  // NOLINT
        }
        {
            auto const start_value = 100;
            auto const delta = 3;
            TestType x{start_value};
            x %= TestType{delta};
            REQUIRE((x == TestType{start_value % delta}));  // NOLINT
        }
    }
    SECTION("Simple arithmetic") {
        {
            auto const start_value = 1;
            auto const delta = 10;
            TestType x{start_value};
            auto const y = x + TestType{delta};
            REQUIRE((y == TestType{start_value + delta}));  // NOLINT
        }
        {
            auto const start_value = 10;
            auto const delta = 5;
            TestType x{start_value};
            auto const y = x - TestType{delta};
            REQUIRE((y == TestType{start_value - delta}));  // NOLINT
        }
        {
            auto const start_value = 2;
            auto const delta = 5;
            TestType x{start_value};
            auto const y = x * TestType{delta};
            REQUIRE((y == TestType{start_value * delta}));  // NOLINT
        }
        {
            auto const start_value = 100;
            auto const delta = 3;
            TestType x{start_value};
            auto const y = x / TestType{delta};
            REQUIRE((y == TestType{start_value / delta}));  // NOLINT
        }
        {
            auto const start_value = 100;
            auto const delta = 3;
            TestType x{start_value};
            auto const y = x % TestType{delta};
            REQUIRE((y == TestType{start_value % delta}));  // NOLINT
        }
    }
}

TEMPLATE_TEST_CASE("Unsigned integrals", "", u8, u16, u32, u64, usize) {  // NOLINT
    using inner_t = detail::inner_type_t<TestType>;

    SECTION("In-place bit operations") {
        SECTION("bitwise and") {
            inner_t const x{0b1010U};
            inner_t const y{0b1000U};
            TestType t{x};
            t &= TestType{y};
            REQUIRE((t == TestType{x & y}));  // NOLINT
        }
        SECTION("bitwise or") {
            inner_t const x{0b1010U};
            inner_t const y{0b0100U};
            TestType t{x};
            t |= TestType{y};
            REQUIRE((t == TestType{x | y}));  // NOLINT
        }
        SECTION("bitwise left shift") {
            inner_t const x{0b0010U};
            inner_t const y{2};
            TestType t{x};
            t <<= TestType{y};
            REQUIRE((t == TestType{x << y}));  // NOLINT
        }
        SECTION("bitwise right shift") {
            inner_t const x{0b1000U};
            inner_t const y{2};
            TestType t{x};
            t >>= TestType{y};
            REQUIRE((t == TestType{x >> y}));  // NOLINT
        }
        SECTION("bitwise xor") {
            inner_t const x{0b1001U};
            inner_t const y{0b1010U};
            TestType t{x};
            t ^= TestType{y};
            REQUIRE((t == TestType{x ^ y}));  // NOLINT
        }
    }
    SECTION("Bit operations") {
        SECTION("bitwise and") {
            inner_t const a{0b1010U};
            inner_t const b{0b1000U};
            TestType x{a};
            TestType y{b};
            REQUIRE(((x & y) == TestType{a & b}));  // NOLINT
        }
        SECTION("bitwise or") {
            inner_t const a{0b1010U};
            inner_t const b{0b0100U};
            TestType x{a};
            TestType y{b};
            REQUIRE(((x | y) == TestType{a | b}));  // NOLINT
        }
        SECTION("bitwise left shift") {
            inner_t const a{0b0010U};
            inner_t const b{2};
            TestType x{a};
            TestType y{b};
            REQUIRE(((x << y) == TestType{a << b}));  // NOLINT
        }
        SECTION("bitwise right shift") {
            inner_t const a{0b1000U};
            inner_t const b{2};
            TestType x{a};
            TestType y{b};
            REQUIRE(((x >> y) == TestType{a >> b}));  // NOLINT
        }
        SECTION("bitwise xor") {
            inner_t const a{0b1001U};
            inner_t const b{0b1010U};
            TestType x{a};
            TestType y{b};
            REQUIRE(((x ^ y) == TestType{a ^ b}));  // NOLINT
        }
        SECTION("bitwise not") {
            inner_t const a{0b1001U};
            TestType x{a};
            REQUIRE((~x == TestType{static_cast<inner_t>(~a)}));  // NOLINT
        }
    }
    SECTION("Wrapping operations") {
        SECTION("Addition") {
            TestType const x{std::numeric_limits<inner_t>::max()};
            TestType const delta{10};
            auto const y = x.wrapping_add(delta);
            REQUIRE((y == delta - TestType{1}));  // NOLINT
        }
        SECTION("Subtraction") {
            TestType const x{std::numeric_limits<inner_t>::min()};
            TestType const delta{10};
            auto const y = x.wrapping_sub(delta);
            REQUIRE((y == TestType{std::numeric_limits<inner_t>::max()} - delta + TestType{1}));  // NOLINT
        }
    }
}

TEST_CASE("Booleans") {  // NOLINT
    SECTION("Truthy") {
        REQUIRE(static_cast<bool>(True));
    }
    SECTION("Falsy") {
        REQUIRE_FALSE(static_cast<bool>(False));
    }
    SECTION("Casting") {
        REQUIRE((True.as<int>() == 1));
        REQUIRE((False.as<int>() == 0));

        REQUIRE((True.as<u8>() == 1_u8));
        REQUIRE((False.as<u8>() == 0_u8));
        REQUIRE((True.as<i8>() == 1_i8));
        REQUIRE((False.as<i8>() == 0_i8));

        REQUIRE((True.as<u16>() == 1_u16));
        REQUIRE((False.as<u16>() == 0_u16));
        REQUIRE((True.as<i16>() == 1_i16));
        REQUIRE((False.as<i16>() == 0_i16));

        REQUIRE((True.as<u32>() == 1_u32));
        REQUIRE((False.as<u32>() == 0_u32));
        REQUIRE((True.as<i32>() == 1_i32));
        REQUIRE((False.as<i32>() == 0_i32));

        REQUIRE((True.as<u64>() == 1_u64));
        REQUIRE((False.as<u64>() == 0_u64));
        REQUIRE((True.as<i64>() == 1_i64));
        REQUIRE((False.as<i64>() == 0_i64));

        REQUIRE((True.as<usize>() == 1_usize));
        REQUIRE((False.as<usize>() == 0_usize));
        REQUIRE((True.as<isize>() == 1_isize));
        REQUIRE((False.as<isize>() == 0_isize));

        REQUIRE((True.as<f32>().val() == 1_a));
        REQUIRE((False.as<f32>().val() == 0_a));
        REQUIRE((True.as<f64>().val() == 1_a));
        REQUIRE((False.as<f64>().val() == 0_a));
    }
}

TEST_CASE("Casting") {  // NOLINT
    SECTION("Same signdneness") {
        SECTION("Unsigned integrals") {
            check_conversion<u8, u16, u32, u64, usize>(10);  // NOLINT
        }
        SECTION("Signed integrals") {
            check_conversion<i8, i16, i32, i64, isize>(10);  // NOLINT
            check_conversion<i8, i16, i32, i64, isize>(-10);  // NOLINT
        }
        SECTION("Signed and unsigned integrals") {
            check_conversion<i8, u16, i32, u64, isize>(10);  // NOLINT
        }
    }
    SECTION("Floating point") {
        SECTION("Floating to integral") {
            REQUIRE(((10.5_f32).as<u8>() == 10_u8));  // NOLINT
            REQUIRE(((10.5_f32).as<u16>() == 10_u16));  // NOLINT
            REQUIRE(((10.5_f64).as<u8>() == 10_u8));  // NOLINT
            REQUIRE(((10.5_f64).as<u16>() == 10_u16));  // NOLINT
        }
        SECTION("Integral to floating") {
            REQUIRE(((10_u8).as<f32>().val() == 10_a));
            REQUIRE(((10_u8).as<f64>().val() == 10_a));

            REQUIRE(((10_i32).as<f32>().val() == 10_a));
            REQUIRE(((10_i32).as<f64>().val() == 10_a));
        }
    }
}

TEMPLATE_TEST_CASE("Floating-point number objects", "", f32, f64) {  // NOLINT
    using type = detail::inner_type_t<TestType>;

    SECTION("Numbers are default constructible") {
        TestType const x{};
        REQUIRE(x.val() == 0_a);  // NOLINT
    }
    SECTION("Numbers are constructible with an argument") {
        for (auto i : {0, 1, 2}) {
            auto const j = static_cast<type>(i);
            TestType const x{j};
            REQUIRE(x.val() == Catch::Approx{j});  // NOLINT
        }
    }
    SECTION("Numbers are copy-constructible") {
        {
            TestType const x{10};
            auto const y{x};
            REQUIRE(x.val() == Catch::Approx{y.val()});  // NOLINT
        }
        {
            TestType const x{10};
            auto const y = x;
            REQUIRE(x.val() == Catch::Approx{y.val()});  // NOLINT
        }
    }
    SECTION("Numbers are copy-assignable") {
        TestType const x{4};
        TestType y{};
        y = x;
        REQUIRE(x.val() == Catch::Approx{y.val()});  // NOLINT
    }
    SECTION("Numbers are comparable") {
        TestType const x{5};
        TestType const y{4};
        REQUIRE((x > y));  // NOLINT
        REQUIRE_FALSE((x < y));  // NOLINT
    }
}

TEST_CASE("Size assertions") {  // NOLINT
    REQUIRE(sizeof(u8) == sizeof(std::uint8_t));
    REQUIRE(sizeof(i8) == sizeof(std::int8_t));
    REQUIRE(sizeof(u16) == sizeof(std::uint16_t));
    REQUIRE(sizeof(i16) == sizeof(std::int16_t));
    REQUIRE(sizeof(u32) == sizeof(std::uint32_t));
    REQUIRE(sizeof(i32) == sizeof(std::int32_t));
    REQUIRE(sizeof(u64) == sizeof(std::uint64_t));
    REQUIRE(sizeof(i64) == sizeof(std::int64_t));
    REQUIRE(sizeof(usize) == sizeof(std::size_t));
    REQUIRE(sizeof(isize) == sizeof(std::ptrdiff_t));
    REQUIRE(sizeof(f32) == sizeof(float));
    REQUIRE(sizeof(f64) == sizeof(double));
    REQUIRE(sizeof(boolean) == sizeof(bool));
}

TEST_CASE("Hashable test") {
    std::unordered_map<u8, int> const m{{0_u8, 0}, {1_u8, 1}};

    for (auto const &[key, value] : m) {
        REQUIRE((key.val() == value));
    }
}

TEMPLATE_TEST_CASE("Trivial type check", "", i8, u8, i16, u16, i32, u32, i64, u64, isize, usize, f32, f64) {  // NOLINT
    REQUIRE((std::is_trivial_v<TestType>));
}

TEMPLATE_TEST_CASE("numeric limits specialization", "", i8, u8, i16, u16, i32, u32, i64, u64, isize, usize, f32, f64) {
    using inner_t = detail::inner_type_t<TestType>;
    static_assert(std::numeric_limits<TestType>::is_specialized == std::numeric_limits<inner_t>::is_specialized);
    static_assert(std::numeric_limits<TestType>::is_signed == std::numeric_limits<inner_t>::is_signed);
    static_assert(std::numeric_limits<TestType>::is_integer == std::numeric_limits<inner_t>::is_integer);
    static_assert(std::numeric_limits<TestType>::is_exact == std::numeric_limits<inner_t>::is_exact);
    static_assert(std::numeric_limits<TestType>::has_infinity == std::numeric_limits<inner_t>::has_infinity);
    static_assert(std::numeric_limits<TestType>::has_quiet_NaN == std::numeric_limits<inner_t>::has_quiet_NaN);
    static_assert(std::numeric_limits<TestType>::has_signaling_NaN == std::numeric_limits<inner_t>::has_signaling_NaN);
    static_assert(std::numeric_limits<TestType>::has_denorm == std::numeric_limits<inner_t>::has_denorm);
    static_assert(std::numeric_limits<TestType>::has_denorm_loss == std::numeric_limits<inner_t>::has_denorm_loss);
    static_assert(std::numeric_limits<TestType>::round_style == std::numeric_limits<inner_t>::round_style);
    static_assert(std::numeric_limits<TestType>::is_iec559 == std::numeric_limits<inner_t>::is_iec559);
    static_assert(std::numeric_limits<TestType>::is_bounded == std::numeric_limits<inner_t>::is_bounded);
    static_assert(std::numeric_limits<TestType>::is_modulo == std::numeric_limits<inner_t>::is_modulo);
    static_assert(std::numeric_limits<TestType>::digits == std::numeric_limits<inner_t>::digits);
    static_assert(std::numeric_limits<TestType>::digits10 == std::numeric_limits<inner_t>::digits10);
    static_assert(std::numeric_limits<TestType>::max_digits10 == std::numeric_limits<inner_t>::max_digits10);
    static_assert(std::numeric_limits<TestType>::radix == std::numeric_limits<inner_t>::radix);
    static_assert(std::numeric_limits<TestType>::min_exponent == std::numeric_limits<inner_t>::min_exponent);
    static_assert(std::numeric_limits<TestType>::min_exponent10 == std::numeric_limits<inner_t>::min_exponent10);
    static_assert(std::numeric_limits<TestType>::max_exponent == std::numeric_limits<inner_t>::max_exponent);
    static_assert(std::numeric_limits<TestType>::max_exponent10 == std::numeric_limits<inner_t>::max_exponent10);
    static_assert(std::numeric_limits<TestType>::traps == std::numeric_limits<inner_t>::traps);
    static_assert(std::numeric_limits<TestType>::tinyness_before == std::numeric_limits<inner_t>::tinyness_before);
    static_assert(std::numeric_limits<TestType>::min().template as<inner_t>() == std::numeric_limits<inner_t>::min());
    static_assert(std::numeric_limits<TestType>::lowest().template as<inner_t>() == std::numeric_limits<inner_t>::lowest());
    static_assert(std::numeric_limits<TestType>::max().template as<inner_t>() == std::numeric_limits<inner_t>::max());
    static_assert(std::numeric_limits<TestType>::epsilon().template as<inner_t>() == std::numeric_limits<inner_t>::epsilon());
    static_assert(std::numeric_limits<TestType>::round_error().template as<inner_t>() == std::numeric_limits<inner_t>::round_error());
    static_assert(std::numeric_limits<TestType>::infinity().template as<inner_t>() == std::numeric_limits<inner_t>::infinity());
    // static_assert(std::numeric_limits<TestType>::quiet_NaN().template as<inner_t>() == std::numeric_limits<inner_t>::quiet_NaN());
    // static_assert(std::numeric_limits<TestType>::signaling_NaN().template as<inner_t>() == std::numeric_limits<inner_t>::signaling_NaN());
    static_assert(std::numeric_limits<TestType>::denorm_min().template as<inner_t>() == std::numeric_limits<inner_t>::denorm_min());
}