#include "cpputils/misc/container_views.hpp"
#include <catch2/catch_all.hpp>


#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>


using namespace cpputils;

namespace {
[[nodiscard]] constexpr auto equals(auto const &r1, auto const &r2) -> bool {
    return std::ranges::equal(r1, r2);
}
}  // namespace

TEST_CASE("as_view test", "[make_views]") {
    SECTION("strings") {
        {
            std::string const s{"Hey"};
            auto const v = as_view(s);
            REQUIRE(v == std::string_view(s));
        }
        {
            const char *s = "Hey";
            auto const v = as_view(s);
            REQUIRE(v == std::string_view(s));
        }
        {
            const wchar_t *s = L"ABCDEF";
            auto const v = as_view(s);
            REQUIRE(v == std::basic_string_view<wchar_t>(s));
        }
    }
    SECTION("dynamic containers") {
        {
            std::vector const vc{1, 2, 3};
            auto const v = as_view(vc);
            REQUIRE(equals(vc, v));
            STATIC_REQUIRE(std::is_same_v<std::span<int const> const, decltype(v)>);
        }
        {
            std::vector const vc{'1', '2', '3'};
            auto const v = as_view(vc);
            REQUIRE(equals(vc, v));
            STATIC_REQUIRE(std::is_same_v<std::string_view const, decltype(v)>);
        }
    }
    SECTION("static containers") {
        {
            auto const a = std::array{1, 2, 3};
            auto const v = as_view(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::span<int const, std::dynamic_extent> const, decltype(v)>);
        }
        {
            auto const a = std::array{'1', '2', '3'};
            auto const v = as_view(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::string_view const, decltype(v)>);
        }
        {
            auto const a = std::array{1, 2, 3};
            auto const v = as_view<with_fixed_extent>(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::span<int const, 3> const, decltype(v)>);
        }
        {
            int const a[] = {1, 2, 3};
            auto const v = as_view(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::span<int const, std::dynamic_extent> const, decltype(v)>);
        }
        {
            char8_t const a[] = {'1', '2', '3'};
            auto const v = as_view(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::basic_string_view<char8_t> const, decltype(v)>);
        }
        {
            int const a[] = {1, 2, 3};
            auto const v = as_view<with_fixed_extent>(a);
            REQUIRE(equals(v, a));
            STATIC_REQUIRE(std::is_same_v<std::span<int const, 3> const, decltype(v)>);
        }
    }
}