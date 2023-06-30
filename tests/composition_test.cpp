// #include "cpputils/functional/composition.hpp"
#include <catch2/catch_all.hpp>

#include "cpputils/functional/composition.hpp"
using namespace cpputils;

namespace {
int div_2(int x) { return x / 2; }
int plus_1(int x) { return x + 1; }
}  // namespace


TEST_CASE("operator-overloading-test", "[composition-test]") {
    {
        auto const r = compose << div_2 << plus_1 | 3;
        REQUIRE(r == 2);
    }
    {
        auto const r = compose >> plus_1 >> div_2 | 3;
        REQUIRE(r == 2);
    }
}

TEST_CASE("compose-left-test", "[composition-test]") {
    auto const left = compose_l(plus_1, div_2);
    REQUIRE(left(3) == div_2(plus_1(3)));
    REQUIRE((left | 3) == div_2(plus_1(3)));
}

TEST_CASE("compose-right-test", "[composition-test]") {
    auto const left = compose_r(div_2, plus_1);
    REQUIRE(left(3) == plus_1(div_2(3)));
    REQUIRE((left | 3) == plus_1(div_2(3)));
}
