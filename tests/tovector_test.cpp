#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/tovector.hpp"
#include <ranges>
#include <vector>


using namespace cpputils;


TEST_CASE("to vector", "[range_to_vector]") {
    std::vector const v{1, 2, 3};
    std::vector const expected{2, 4, 6};
    auto mult = v | std::views::transform([](int i) { return i * 2; }) | to_vector();

    for (std::size_t i{}; i < mult.size(); ++i) {
        REQUIRE(mult[i] == expected[i]);
    }
}

TEST_CASE("to vector", "[empty_case]") {
    std::vector const v{1, 2, 3};
    auto mult = v | std::views::filter([](int i) { return i > 10; }) | to_vector();

    REQUIRE(mult.empty());
}
