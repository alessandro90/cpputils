#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/zip.hpp"
#include "cpputils/functional/zip_with.hpp"
#include <vector>


using namespace cpputils;

namespace {
auto const sum = [](auto... vs) { return (vs + ...); };
}

TEST_CASE("zip_with test", "[same_length]") {
    std::vector v{1, 2, 3};
    int arr[] = {10, 20, 30};

    std::size_t i{};
    for (auto item : zip_with(sum, v, arr)) {
        REQUIRE(item == v[i] + arr[i]);
        ++i;
    }
}

TEST_CASE("zip_with test", "[different_length]") {
    std::vector v{1, 2, 3, 4};
    int arr[] = {10, 20, 30};

    std::size_t i{};
    for (auto item : zip_with(sum, v, arr)) {
        REQUIRE(item == v[i] + arr[i]);
        ++i;
    }
}

TEST_CASE("zip_with test", "[can_be_piped]") {
    std::vector v{1, 2, 3};
    int arr[] = {10, 20, 30};
    int expected[] = {11, 33};
    auto out = zip_with(sum, v, arr)
               | std::ranges::views::filter([](int i) { return i % 2 != 0; });

    std::size_t i{};
    for (auto j : out) {
        REQUIRE(j == expected[i]);
        ++i;
    }
}
