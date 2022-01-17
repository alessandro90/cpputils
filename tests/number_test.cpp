#include "cpputils/types/number.hpp"
#include <catch2/catch_all.hpp>
#include <ranges>


using namespace cpputils;
using namespace Catch::literals;

// NOLINTNEXTLINE
// TEMPLATE_TEST_CASE("Integral number objects", "", i8, u8, i16, u16, i32, u32, i64, u64, isize, usize) {
TEMPLATE_TEST_CASE("Integral number objects", "", i32) {
    using type = detail::inner_type_t<TestType>;

    SECTION("Numbers are default constructible") {
        TestType const x{};
        REQUIRE(x.val() == 0);
    }
    SECTION("Numbers are constructible with an argument") {
        for (auto i : std::views::iota(0, 3)) {
            auto const j = static_cast<type>(i);
            TestType const x{j};
            REQUIRE(x.val() == j);
        }
    }
    SECTION("Numbers are copy-constructible") {
        {
            TestType const x{10};
            auto const y{x};
            REQUIRE(x.val() == y.val());
        }
        {
            TestType const x{10};
            auto const y = x;
            REQUIRE(x.val() == y.val());
        }
    }
    SECTION("Numbers are copy-assignable") {
        TestType const x{4};
        TestType y{};
        y = x;
        REQUIRE(x.val() == y.val());
    }
    SECTION("Numbers are comparable") {
        {
            TestType const x{5};
            TestType const y{4};
            REQUIRE((x >= y));
            REQUIRE((x != y));

            REQUIRE_FALSE((x < y));
            REQUIRE_FALSE((x <= y));
            REQUIRE_FALSE((x == y));
        }
        {
            TestType const x{9};
            TestType const y{x};
            REQUIRE((x == y));
            REQUIRE_FALSE((x != y));
        }
    }
}

TEMPLATE_TEST_CASE("Floating-point number objects", "", f32, f64) {
    using type = detail::inner_type_t<TestType>;

    SECTION("Numbers are default constructible") {
        TestType const x{};
        REQUIRE(x.val() == 0_a);
    }
    SECTION("Numbers are constructible with an argument") {
        for (auto i : std::views::iota(0, 3)) {
            auto const j = static_cast<type>(i);
            TestType const x{j};
            REQUIRE(x.val() == Catch::Approx{j});
        }
    }
    SECTION("Numbers are copy-constructible") {
        {
            TestType const x{10};
            auto const y{x};
            REQUIRE(x.val() == Catch::Approx{y.val()});
        }
        {
            TestType const x{10};
            auto const y = x;
            REQUIRE(x.val() == Catch::Approx{y.val()});
        }
    }
    SECTION("Numbers are copy-assignable") {
        TestType const x{4};
        TestType y{};
        y = x;
        REQUIRE(x.val() == Catch::Approx{y.val()});
    }
    SECTION("Numbers are comparable") {
        TestType const x{5};
        TestType const y{4};
        REQUIRE((x > y));
        REQUIRE_FALSE((x < y));
    }
}
