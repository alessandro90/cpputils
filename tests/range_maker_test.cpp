#include "cpputils/functional/utils.hpp"
#include <catch2/catch_all.hpp>

#include <array>
#include <list>
#include <map>
#include <vector>


using namespace cpputils;

TEST_CASE("make-vector", "[range-maker-test]") {
    auto const v = make_range<std::vector<int>>([n = 0]() mutable { return n++; }, 3U);
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 1);
    REQUIRE(v[2] == 2);
}

TEST_CASE("make-list", "[range-maker-test]") {
    auto const v = make_range<std::list<int>>([n = 0]() mutable { return n++; }, 3U);
    REQUIRE(v.size() == 3);
    int current{};
    for (auto i : v) {
        REQUIRE(i == current);
        ++current;
    }
}

TEST_CASE("make-map", "[range-maker-test]") {
    using value_type = std::map<int, int>::value_type;
    auto const v = make_range<std::map<int, int>>([n = 0]() mutable { return value_type{n++, n}; }, 3U);
    REQUIRE(v.size() == 3);
    REQUIRE(v.at(0) == 1);
    REQUIRE(v.at(1) == 2);
    REQUIRE(v.at(2) == 3);
}

TEST_CASE("make-array", "[range-maker-test]") {
    constexpr auto v = make_range<std::array<int, 3>>([n = 0]() mutable { return n++; });
    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 1);
    REQUIRE(v[2] == 2);
}
