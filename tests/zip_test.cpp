#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/zip.hpp"
#include <cstddef>
#include <ranges>
#include <vector>


using namespace cpputils;


TEST_CASE("zip test", "[same_length]") {
    std::vector v{1, 2, 3};
    int arr[] = {10, 20, 30};

    std::size_t i{};
    for (auto [v_item, arr_item] : zip(v, arr)) {
        REQUIRE(v_item == v[i]);
        REQUIRE(arr_item == arr[i]);
        ++i;
    }
}

TEST_CASE("zip test", "[different_length]") {
    std::vector v{1, 2, 3, 4};
    int arr[] = {10, 20, 30};

    std::size_t i{};
    for (auto [v_item, arr_item] : zip(v, arr)) {
        REQUIRE(v_item == v[i]);
        REQUIRE(arr_item == arr[i]);
        ++i;
    }
}

TEST_CASE("zip test", "[mutate_reference]") {
    std::vector v{1, 2, 3};
    int arr[] = {10, 20, 30};

    std::size_t i{};
    for (auto [v_item, arr_item] : zip(v, arr)) {
        ++v_item;
        ++arr_item;
        REQUIRE(v_item == v[i]);
        REQUIRE(arr_item == arr[i]);
        ++i;
    }
}


TEST_CASE("zip test", "[can_be_piped]") {
    std::vector v{1, 2, 3};
    int arr[] = {10, 20, 30};

    auto sum = zip(v, arr)
               | std::ranges::views::transform([](auto pair) {
                     return std::get<0>(pair) + std::get<1>(pair);
                 });

    std::size_t i{};
    for (auto j : sum) {
        REQUIRE(j == v[i] + arr[i]);
        ++i;
    }
}
