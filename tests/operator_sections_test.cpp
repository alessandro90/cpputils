#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/operator_sections.hpp"

using cpputils::_;  // NOLINT

// NOLINTNEXTLINE
TEST_CASE("sections test", "") {
    SECTION("operator +") {
        auto const sum = _ + _;
        REQUIRE(sum(1, 2) == 3);
    }
    SECTION("partial operator +") {
        {
            auto const add_10 = _ + 10;
            REQUIRE(add_10(1) == 11);
        }
        {
            auto const add_10 = 10 + _;
            REQUIRE(add_10(1) == 11);
        }
    }
    ///
    SECTION("operator -") {
        auto const subtract = _ - _;
        REQUIRE(subtract(1, 2) == -1);
    }
    SECTION("partial operator -") {
        {
            auto const subtract_10 = _ - 10;
            REQUIRE(subtract_10(1) == -9);
        }
        {
            auto const subtract_10 = 10 - _;
            REQUIRE(subtract_10(10) == 0);
        }
    }
    SECTION("Unary minus") {
        auto const negate = -_;
        REQUIRE(negate(1) == -1);
    }
}