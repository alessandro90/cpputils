#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/enumerate.hpp"
#include "cpputils/types/number.hpp"
#include <cstddef>
#include <ranges>
#include <vector>


using namespace cpputils;
using namespace cpputils::literals;


TEST_CASE("enumerate test", "[from_vector]") {
    {
        std::vector const v{10, 20, 30};
        auto e = v | enumerate();  // NOLINT
        std::size_t expected_index{};
        for (auto [index, value] : e) {
            REQUIRE(static_cast<std::size_t>(index) == expected_index);
            REQUIRE(value == v[expected_index]);
            ++expected_index;
        }
    }
    {
        std::vector const v{10, 20, 30};
        auto e = v | enumerate(1);  // NOLINT
        std::size_t expected_index{};
        for (auto [index, value] : e) {
            REQUIRE(static_cast<std::size_t>(index - 1) == expected_index);
            REQUIRE(value == v[expected_index]);
            ++expected_index;
        }
    }
    {
        std::vector const v{10, 20, 30};
        auto e = v | enumerate(-1);  // NOLINT
        std::size_t expected_index{};
        for (auto [index, value] : e) {
            REQUIRE(static_cast<std::size_t>(index + 1) == expected_index);
            REQUIRE(value == v[expected_index]);
            ++expected_index;
        }
    }
    {
        std::vector const v{10, 20, 30};
        auto e = v | enumerate(std::size_t{});  // NOLINT
        std::size_t expected_index{};
        for (auto [index, value] : e) {
            REQUIRE(index == expected_index);
            REQUIRE(value == v[expected_index]);
            ++expected_index;
        }
    }
    {
        std::vector const v{10, 20, 30};
        auto e = v | enumerate(0_u8);  // NOLINT
        std::size_t expected_index{};
        for (auto [index, value] : e) {
            REQUIRE((index.as<std::size_t>() == expected_index));
            REQUIRE(value == v[expected_index]);
            ++expected_index;
        }
    }
}

TEST_CASE("enumerate test", "[ranges_compatible]") {
    std::vector v{10, 20, 30};
    auto out = v
               | std::ranges::views::transform([](int i) { return i * 2; })
               | enumerate()
               | std::ranges::views::filter([](auto element) { return element.value > 20; });

    std::size_t i{1};
    for (auto [index, value] : out) {
        REQUIRE(index == i);
        REQUIRE(value == v[i] * 2);
        ++i;
    }
}

TEST_CASE("enumerate test", "[reference_test]") {
    std::vector v{10, 20, 30};
    std::vector const expected{20, 40, 60};
    auto e = v | enumerate();  // NOLINT
    for (auto [index, value] : e) {
        value *= 2;
    }
    for (std::size_t i{}; i < v.size(); ++i) {
        REQUIRE(v[i] == expected[i]);
    }
}
