#include "cpputils/meta/traits.hpp"
#include <catch2/catch_all.hpp>


using namespace cpputils;

namespace {
template <typename... T>
void h(T...);

void call(float) {}  // NOLINT

struct S {
    explicit S(int) {}
    void call(float) {}
};
auto const f_0 = []() {};  // NOLINT
auto const f_1 = [](int) {};  // NOLINT
auto const f_s_l = [](S &) {};  // NOLINT
auto const f_s_p = [](S *) {};  // NOLINT
auto const f_s_r = [](S &&) {};  // NOLINT
auto const f_s_l_c = [](S const &) {};  // NOLINT
auto const f_v = [](auto &&...) {};  // NOLINT
auto const f_4 = [](int, int, int, int) {};  // NOLINT
}  // namespace

TEST_CASE("arity-check-compile", "[should-compile]") {
    STATIC_REQUIRE(*arity_v<decltype(f_s_p)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(f_0)> == 0);
    STATIC_REQUIRE(*arity_v<decltype(f_1)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(f_s_l)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(f_s_r)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(f_s_l_c)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(h<int, float>)> == 2);
    STATIC_REQUIRE(*arity_v<decltype(f_v)> == 0);
    STATIC_REQUIRE(*arity_v<decltype(&S::call)> == 2);
    STATIC_REQUIRE(*arity_v<decltype(call)> == 1);
    STATIC_REQUIRE(*arity_v<decltype(&call)> == 1);
    STATIC_REQUIRE(!arity_v<decltype(f_4), 3>);
}