#include "catch2/catch_test_macros.hpp"
#include "functional/enumerate.hpp"
#include "functional/opt_ext.hpp"
#include "functional/tovector.hpp"
#include "types/number.hpp"
#include <iostream>
#include <vector>

using namespace cpputils;

// TEST_CASE("Enumerate construction", "[build]") {
//     // auto const v = std::optional{5} >> [](int x) { return x * 2; };
//     auto const v = map([](int i) { return i + 2; }, std::optional{5});
//     auto const p = v
//                    >> pipe([](int i) { return i; })
//                    >> or_else([]() { std::cout << "Empty optional\n"; })
//                    >> unwrap_or_else([]() { return 1; });


//     REQUIRE(p == 7);
// }

TEST_CASE("Enumerate pipe", "[pipe]") {
    std::vector v{1, 23, 3};
    for (auto [i, val] : v | enumerate(0) | to_vector()) {
        std::cout << "val: " << val << "; i: " << i << '\n';
    }

    for (auto i : v | to_vector()) {
        std::cout << "i: " << i << '\n';
    }

    REQUIRE(1 == 1);
}

TEST_CASE("number literal", "[literal]") {
    auto const i = 10_u32;
    REQUIRE(1 == 1);
}