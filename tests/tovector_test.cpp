#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/tovector.hpp"
#include <ranges>
#include <vector>


using namespace cpputils;


TEST_CASE("to vector", "[literal]") {
    std::vector v{1, 2, 3};
    auto mult = v | std::views::transform([](int i) { return i * 2; }) | to_vector();
    REQUIRE(1 == 1);
}