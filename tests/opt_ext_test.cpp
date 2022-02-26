#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/opt_ext.hpp"
#include <memory>
#include <optional>
#include <tuple>


using namespace cpputils;

namespace {
using intopt = std::optional<int>;

auto const increment = [](int i) -> int { return i + 1; };
auto const sum = [](int a, int b) -> int { return a + b; };

auto const increment_opt = [](int i) -> intopt { return {i + 1}; };
auto const sum_opt = [](int a, int b) -> intopt { return {a + b}; };

auto const return_v = [](int v) { return [v]() { return v; }; };

auto const increase = [](auto &...vs) { (++vs, ...); };
auto const increase_ref = [](int &v) { return [&v]() { ++v; }; };
}  // namespace

TEST_CASE("opt test", "[map]") {  // NOLINT
    {
        auto const out = map(increment, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(increment, intopt{0});
        REQUIRE(out.has_value());
        REQUIRE(out.value() == 1);
    }
    {
        auto const out = map(sum, intopt{}, intopt{1});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum, intopt{1}, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum, intopt{}, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum, intopt{1}, intopt{2});
        REQUIRE(out.has_value());
        REQUIRE(out.value() == 3);
    }

    //
    {
        auto const out = map(increment_opt, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(increment_opt, intopt{0});
        REQUIRE(out.has_value());
        REQUIRE(out.value() == 1);
    }
    {
        auto const out = map(sum_opt, intopt{}, intopt{1});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum_opt, intopt{1}, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum_opt, intopt{}, intopt{});
        REQUIRE(!out.has_value());
    }
    {
        auto const out = map(sum_opt, intopt{1}, intopt{2});
        REQUIRE(out.has_value());
        REQUIRE(out.value() == 3);
    }
}

TEST_CASE("opt test", "[try_or]") {  // NOLINT
    {
        auto const out = try_or(increment, 0, intopt{});
        REQUIRE(out == 0);
    }
    {
        auto const out = try_or(increment, 0, intopt{1});
        REQUIRE(out == 2);
    }
    {
        auto const out = try_or(sum, -1, intopt{}, intopt{});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or(sum, -1, intopt{}, intopt{0});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or(sum, -1, intopt{0}, intopt{});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or(sum, -1, intopt{1}, intopt{1});
        REQUIRE(out == 2);
    }
}

TEST_CASE("opt test", "[try_or_else]") {  // NOLINT
    {
        auto const out = try_or_else(increment, return_v(0), intopt{});
        REQUIRE(out == 0);
    }
    {
        auto const out = try_or_else(increment, return_v(0), intopt{1});
        REQUIRE(out == 2);
    }
    {
        auto const out = try_or_else(sum, return_v(-1), intopt{}, intopt{});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or_else(sum, return_v(-1), intopt{}, intopt{0});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or_else(sum, return_v(-1), intopt{0}, intopt{});
        REQUIRE(out == -1);
    }
    {
        auto const out = try_or_else(sum, return_v(-1), intopt{1}, intopt{1});
        REQUIRE(out == 2);
    }
}

TEST_CASE("opt test", "[apply]") {  // NOLINT
    {
        intopt a{};
        intopt b{};
        apply(increase, a, b);
        REQUIRE(!a.has_value());
        REQUIRE(!b.has_value());
    }
    {
        intopt a{0};
        intopt b{};
        apply(increase, a, b);
        REQUIRE(a.has_value());
        REQUIRE(a.value() == 0);
        REQUIRE(!b.has_value());
    }
    {
        intopt a{};
        intopt b{0};
        apply(increase, a, b);
        REQUIRE(!a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(b.value() == 0);
    }
    {
        intopt a{1};
        intopt b{2};
        apply(increase, a, b);
        REQUIRE(a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(a.value() == 2);
        REQUIRE(b.value() == 3);
    }
    {
        intopt a{1};
        intopt b{2};
        intopt c{10};
        apply(increase, a, b, c);
        REQUIRE(a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(c.has_value());
        REQUIRE(a.value() == 2);
        REQUIRE(c.value() == 11);
    }
}

TEST_CASE("opt test", "[apply_or_else]") {  // NOLINT
    {
        intopt a{};
        intopt b{};
        int v{};
        apply_or_else(increase, increase_ref(v), a, b);
        REQUIRE(!a.has_value());
        REQUIRE(!b.has_value());
        REQUIRE(v == 1);
    }
    {
        intopt a{0};
        intopt b{};
        int v{};
        apply_or_else(increase, increase_ref(v), a, b);
        REQUIRE(a.has_value());
        REQUIRE(a.value() == 0);
        REQUIRE(!b.has_value());
        REQUIRE(v == 1);
    }
    {
        intopt a{};
        intopt b{0};
        int v{};
        apply_or_else(increase, increase_ref(v), a, b);
        REQUIRE(!a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(b.value() == 0);
        REQUIRE(v == 1);
    }
    {
        intopt a{1};
        intopt b{2};
        int v{};
        apply_or_else(increase, increase_ref(v), a, b);
        REQUIRE(a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(a.value() == 2);
        REQUIRE(b.value() == 3);
        REQUIRE(v == 0);
    }
    {
        intopt a{1};
        intopt b{2};
        intopt c{10};
        int v{};
        apply_or_else(increase, increase_ref(v), a, b, c);
        REQUIRE(a.has_value());
        REQUIRE(b.has_value());
        REQUIRE(c.has_value());
        REQUIRE(a.value() == 2);
        REQUIRE(c.value() == 11);
        REQUIRE(v == 0);
    }
}

TEST_CASE("opt test", "[transform]") {  // NOLINT
    {
        auto const out = intopt{0}
                         >> transform(increment)
                         >> transform(increment_opt);
        REQUIRE(out.has_value());
        REQUIRE(out.value() == 2);
    }
    {
        auto const out = intopt{}
                         >> transform(increment)
                         >> transform(increment_opt);
        REQUIRE(!out.has_value());
    }
    {
        auto const out = intopt{0}
                         >> transform(increment)
                         >> transform([](int x) {
                               if (x == 1) { return intopt{}; }
                               return intopt{2};
                           });
        REQUIRE(!out.has_value());
    }
}

TEST_CASE("opt test", "[if_value]") {  // NOLINT
    {
        intopt a{};
        auto const &b = a >> if_value(increase);
        REQUIRE(!a.has_value());
        REQUIRE(&a == &b);
    }
    {
        intopt a{0};
        auto const &b = a >> if_value(increase);
        REQUIRE(a.has_value());
        REQUIRE(a.value() == 1);
        REQUIRE(&a == &b);
    }
}

TEST_CASE("opt test", "[or_else]") {  // NOLINT
    {
        intopt a{};
        int v{};
        auto const &b = a >> or_else(increase_ref(v));
        REQUIRE(&a == &b);
        REQUIRE(v == 1);
        REQUIRE(!a.has_value());
    }
    {
        intopt a{0};
        int v{};
        auto const &b = a >> or_else(increase_ref(v));
        REQUIRE(&a == &b);
        REQUIRE(v == 0);
        REQUIRE(a.has_value());
        REQUIRE(a.value() == 0);
    }
}

TEST_CASE("opt test", "[unwrap]") {  // NOLINT
    {
        intopt a{};
        try {
            std::ignore = a >> unwrap();
            FAIL("You should not get here");
        } catch (std::bad_optional_access const &) {
            SUCCEED("");
        }
    }
    {
        intopt a{1};
        auto const &v = a >> unwrap();
        REQUIRE(&v == std::addressof(a.value()));
    }
}


TEST_CASE("opt test", "[unwrap_or]") {  // NOLINT
    {
        intopt a{};
        auto const b = a >> unwrap_or(1);
        REQUIRE(b == 1);
    }
    {
        intopt a{1};
        auto const b = a >> unwrap_or(-1);
        REQUIRE(b == 1);
    }
}

TEST_CASE("opt test", "[unwrap_or_else]") {  // NOLINT
    {
        intopt a{};
        auto const b = a >> unwrap_or_else(return_v(1));
        REQUIRE(b == 1);
    }
    {
        intopt a{1};
        auto const b = a >> unwrap_or_else(return_v(-1));
        REQUIRE(b == 1);
    }
}
