#include "catch2/catch_test_macros.hpp"
#include "types/number.hpp"
#include <vector>

using namespace cpputils;


TEST_CASE("number literal", "[literal]") {
    auto const i = 10_u32;
    REQUIRE(1 == 1);
}