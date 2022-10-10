#include "catch2/catch_test_macros.hpp"
#include "cpputils/meta/traits.hpp"
#include "cpputils/types/optional_ref.hpp"
#include "test_utils.hpp"
#include <optional>
#include <type_traits>


using namespace cpputils;
using namespace cpputils::test_utils;

namespace {
struct S {
    int v;
};


}  // namespace

TEST_CASE("is-optional-like", "[optional-ref-test]") {
    STATIC_REQUIRE(optional_like<optional_ref<int>>);
    STATIC_REQUIRE(optional_like<optional_ref<int> &&>);
    STATIC_REQUIRE(optional_like<optional_ref<int> &>);
    STATIC_REQUIRE(optional_like<optional_ref<int> const &>);
    STATIC_REQUIRE(optional_like<optional_ref<int const>>);
    STATIC_REQUIRE(optional_like<optional_ref<int const> &&>);
    STATIC_REQUIRE(optional_like<optional_ref<int const> &>);
    STATIC_REQUIRE(optional_like<optional_ref<int const> const &>);
}

// NOLINTNEXTLINE
TEST_CASE("construction-and-accessors-test", "[optional-ref-test]") {
    {
        optional_ref<int> opt{};
        REQUIRE_FALSE(opt.has_value());
        REQUIRE_FALSE(opt);
        REQUIRE_THROWS_AS(opt.value(), bad_optional_ref_access);
        REQUIRE_THROWS_AS(*opt, bad_optional_ref_access);
    }
    {
        optional_ref<int> opt{std::nullopt};
        REQUIRE_FALSE(opt.has_value());
        REQUIRE_FALSE(opt);
        REQUIRE_THROWS_AS(opt.value(), bad_optional_ref_access);
        REQUIRE_THROWS_AS(*opt, bad_optional_ref_access);
    }
    {
        int x{10};  // NOLINT
        optional_ref opt{x};
        REQUIRE(opt.has_value());
        REQUIRE(opt.value() == 10);
        REQUIRE(*opt == 10);
        STATIC_REQUIRE(std::is_same_v<decltype(opt)::value_type, int>);
    }
    {
        int const x{10};
        optional_ref opt{x};
        REQUIRE(opt.has_value());
        REQUIRE(opt.value() == 10);
        REQUIRE(*opt == 10);
        STATIC_REQUIRE(std::is_same_v<decltype(opt)::value_type, int const>);
    }
    {
        S const s{10};
        optional_ref opt{s};
        REQUIRE(opt->v == 10);
    }
}

TEST_CASE("modifiers-test", "[optional-ref-test]") {
    {
        int const x{10};
        optional_ref opt{x};
        opt.reset();
        REQUIRE_FALSE(opt.has_value());
    }
    {
        int const x{10};
        int const y{11};
        optional_ref opt{x};
        opt = y;
        REQUIRE(opt.value() == y);
    }
    {
        int const x{10};
        int const y{11};
        optional_ref opt_1{x};
        optional_ref opt_2{y};
        swap(opt_1, opt_2);
        REQUIRE(opt_1.value() == y);
        REQUIRE(opt_2.value() == x);
    }
    {
        int const x{10};
        int const y{11};
        optional_ref opt_1{x};
        optional_ref opt_2{y};
        opt_1.swap(opt_2);
        REQUIRE(opt_1.value() == y);
        REQUIRE(opt_2.value() == x);
    }
}

TEST_CASE("conversion-to-std-optional-test", "[optional-ref-test]") {
    {
        int const x{10};
        optional_ref const opt{x};
        auto const stdopt = opt.as_owned();
        REQUIRE(stdopt.value() == x);  // NOLINT
    }
    {
        optional_ref<int> const opt{};
        auto const stdopt = opt.as_owned();
        REQUIRE_FALSE(stdopt.has_value());
    }
    {
        move_detector mv{};
        optional_ref opt{mv};
        auto const stdopt = opt.take_ownership();
        REQUIRE(mv.moved);
        REQUIRE(stdopt.has_value());
    }
}

// NOLINTNEXTLINE
TEST_CASE("comparison-test", "[optional-ref-test]") {
    SECTION("equality-test") {
        {
            int const x{};
            REQUIRE(optional_ref<int>{} == std::nullopt);
            REQUIRE(std::nullopt == optional_ref<int>{});
            REQUIRE(optional_ref{x} != std::nullopt);
            REQUIRE(std::nullopt != optional_ref{x});
        }
        {
            int const x{10};
            int const y{10};
            REQUIRE(optional_ref{x} == x);
            REQUIRE(x == optional_ref{x});
            REQUIRE(optional_ref{x} == optional_ref{y});
        }
        {
            int const x{10};
            int const y{};
            REQUIRE(optional_ref{y} != x);
            REQUIRE(y != optional_ref{x});
            REQUIRE(optional_ref{x} != optional_ref{y});
        }
        {
            int const x{10};
            REQUIRE(std::optional{x} == optional_ref{x});
            REQUIRE(optional_ref{x} == std::optional{x});
        }
        {
            int const x{10};
            REQUIRE(std::optional{x + 1} != optional_ref{x});
            REQUIRE(optional_ref{x} != std::optional{x + 1});
        }
    }
    SECTION("inequality-test") {
        {
            int const x{};
            REQUIRE(optional_ref<int>{} <= std::nullopt);
            REQUIRE(optional_ref<int>{} >= std::nullopt);
            REQUIRE_FALSE(optional_ref<int>{} > std::nullopt);
            REQUIRE_FALSE(optional_ref<int>{} < std::nullopt);
            REQUIRE(std::nullopt <= optional_ref<int>{});
            REQUIRE(std::nullopt >= optional_ref<int>{});
            REQUIRE_FALSE(std::nullopt > optional_ref<int>{});
            REQUIRE_FALSE(std::nullopt < optional_ref<int>{});
            REQUIRE(optional_ref{x} >= std::nullopt);
            REQUIRE(optional_ref{x} > std::nullopt);
            REQUIRE(std::nullopt <= optional_ref{x});
            REQUIRE(std::nullopt < optional_ref{x});
        }
        {
            int const x{10};
            int const y{10};
            REQUIRE(optional_ref{x} >= x);
            REQUIRE(optional_ref{x} <= x);
            REQUIRE_FALSE(optional_ref{x} > x);
            REQUIRE_FALSE(optional_ref{x} < x);
            REQUIRE(x >= optional_ref{x});
            REQUIRE(x <= optional_ref{x});
            REQUIRE_FALSE(x < optional_ref{x});
            REQUIRE_FALSE(x > optional_ref{x});
            REQUIRE(optional_ref{x} >= optional_ref{y});
            REQUIRE(optional_ref{x} <= optional_ref{y});
            REQUIRE_FALSE(optional_ref{x} < optional_ref{y});
            REQUIRE_FALSE(optional_ref{x} > optional_ref{y});
        }
        {
            int const x{10};
            int const y{0};
            REQUIRE(optional_ref{y} < x);
            REQUIRE_FALSE(optional_ref{y} > x);
            REQUIRE(optional_ref{y} <= x);
            REQUIRE_FALSE(optional_ref{y} >= x);
            REQUIRE(x > optional_ref{y});
            REQUIRE(x >= optional_ref{y});
            REQUIRE(optional_ref{y} <= optional_ref{x});
            REQUIRE(optional_ref{y} < optional_ref{x});
            REQUIRE_FALSE(optional_ref{y} >= optional_ref{x});
            REQUIRE_FALSE(optional_ref{y} > optional_ref{x});
        }
        {
            int const x{10};
            REQUIRE(std::optional{x + 1} >= optional_ref{x});
            REQUIRE(optional_ref{x} <= std::optional{x + 1});
        }
    }
}
