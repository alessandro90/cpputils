#include "cpputils/functional/composition.hpp"
#include <catch2/catch_all.hpp>

using namespace cpputils;

namespace {
int div_2(int x) { return x / 2; }
int plus_1(int x) { return x + 1; }
}  // namespace

TEST_CASE("composition-test", "[operator-overloading-test]") {
    {
        auto const r = compose << div_2 << plus_1 | 3;
        REQUIRE(r == 2);
    }
    {
        auto const r = compose >> plus_1 >> div_2 | 3;
        REQUIRE(r == 2);
    }
}
