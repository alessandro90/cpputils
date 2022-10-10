#include "cpputils/functional/expected.hpp"
#include "test_utils.hpp"
#include <catch2/catch_all.hpp>
#include <compare>
#include <type_traits>


using namespace cpputils;
using namespace ::cpputils::test_utils;

namespace {
struct trivial {};

struct copy_trivial {
    copy_trivial(copy_trivial const &) = default;
    copy_trivial &operator=(copy_trivial const &) = default;

    copy_trivial(copy_trivial &&) noexcept {}
    copy_trivial &operator=(copy_trivial &&) noexcept { return *this; }

    ~copy_trivial() = default;
};

struct move_trivial {
    move_trivial(move_trivial const &) {}  // NOLINT
    move_trivial &operator=(move_trivial const &) { return *this; }  // NOLINT

    move_trivial(move_trivial &&) = default;
    move_trivial &operator=(move_trivial &&) = default;

    ~move_trivial() = default;
};

struct destroy_trivial {
    destroy_trivial(destroy_trivial const &) {}  // NOLINT
    destroy_trivial &operator=(destroy_trivial const &) { return *this; }  // NOLINT

    destroy_trivial(destroy_trivial &&) noexcept {}
    destroy_trivial &operator=(destroy_trivial &&) noexcept { return *this; }

    ~destroy_trivial() = default;
};

struct not_trivial {
    not_trivial(not_trivial const &) {}  // NOLINT
    not_trivial &operator=(not_trivial const &) { return *this; }  // NOLINT

    not_trivial(not_trivial &&) noexcept {}
    not_trivial &operator=(not_trivial &&) noexcept { return *this; }

    ~not_trivial() noexcept {}  // NOLINT
};

struct throwing {
    throwing(throwing const &) {}  // NOLINT
    throwing &operator=(throwing const &) { return *this; }  // NOLINT

    throwing(throwing &&) {}  // NOLINT
    throwing &operator=(throwing &&) { return *this; }  // NOLINT

    ~throwing() noexcept(false) {}  // NOLINT
};
struct no_copy_move {
    no_copy_move(no_copy_move const &) = delete;
    no_copy_move &operator=(no_copy_move const &) = delete;

    no_copy_move(no_copy_move &&) = delete;
    no_copy_move &operator=(no_copy_move &&) = delete;

    ~no_copy_move() = default;
};

struct composite_value {
    int a;
    int b;

    constexpr auto operator<=>(composite_value const &) const = default;
};

struct composite_error {
    int a;
    char b;
    constexpr auto operator<=>(composite_error const &) const = default;
};

struct ub_error_policy {
    static void bad_value_access() {}
    static void bad_error_access() {}
};
}  // namespace

TEST_CASE("expected", "[type_properties]") {  // NOLINT
    {
        using trivial_expected = expected<trivial, trivial>;
        STATIC_REQUIRE(std::is_trivially_copy_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_copy_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_destructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_move_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_destructible_v<trivial_expected>);
    }
    {
        using trivial_expected = expected<not_trivial &, not_trivial &>;
        STATIC_REQUIRE(std::is_trivially_copy_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_copy_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_destructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_move_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_nothrow_destructible_v<trivial_expected>);
    }
    {
        using throwing_expected = expected<throwing, throwing>;
        STATIC_REQUIRE(!std::is_nothrow_move_assignable_v<throwing_expected>);
        STATIC_REQUIRE(!std::is_nothrow_move_constructible_v<throwing_expected>);
        STATIC_REQUIRE(!std::is_nothrow_destructible_v<throwing_expected>);
    }
    {
        using no_copy_move_expected = expected<no_copy_move, no_copy_move>;
        STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<no_copy_move_expected>);
        STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<no_copy_move_expected>);
        STATIC_REQUIRE(std::is_trivially_destructible_v<no_copy_move_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_assignable_v<no_copy_move_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_constructible_v<no_copy_move_expected>);
    }
    {
        using trivial_expected = expected<copy_trivial, copy_trivial>;
        STATIC_REQUIRE(std::is_trivially_copy_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_copy_constructible_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_constructible_v<trivial_expected>);
    }
    {
        using trivial_expected = expected<move_trivial, move_trivial>;
        STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(std::is_trivially_move_constructible_v<trivial_expected>);
    }
    {
        using trivial_expected = expected<move_trivial, copy_trivial>;
        STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_assignable_v<trivial_expected>);
        STATIC_REQUIRE(!std::is_trivially_move_constructible_v<trivial_expected>);
    }
}

TEST_CASE("expected", "[build and equality checks]") {  // NOLINT
    {
        using integral_exp_t = expected<int, char>;
        {
            auto const e1 = integral_exp_t{flag_value, 10};
            auto const e2 = integral_exp_t::val(10);
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = integral_exp_t{flag_value, 10};
            auto const e2 = integral_exp_t::val(9);
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = integral_exp_t{flag_error, 'a'};
            auto const e2 = integral_exp_t::val(10);
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = integral_exp_t{flag_value, 10};
            auto const e2 = integral_exp_t::err('p');
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = integral_exp_t{flag_error, 'a'};
            auto const e2 = integral_exp_t::err('a');
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = integral_exp_t{flag_error, 'a'};
            auto const e2 = integral_exp_t::err('b');
            REQUIRE(e1 != e2);
        }
    }
    {
        using composite_exp_t = expected<composite_value, composite_error>;
        {
            auto const e1 = composite_exp_t{flag_value, std::in_place, 10, 40};
            auto const e2 = composite_exp_t::val(composite_value{10, 40});
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = composite_exp_t{flag_error, std::in_place, 10, 'b'};
            auto const e2 = composite_exp_t::err(composite_error{10, 'b'});
            REQUIRE(e1 == e2);
        }
    }
    {
        using throw_exp_t = expected<int, char, bad_expected_access_throw_policy>;
        using nothrow_exp_t = expected<int, char, ub_error_policy>;
        {
            auto const e1 = throw_exp_t{flag_value, 10};
            nothrow_exp_t const e2{e1};
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = throw_exp_t{flag_value, 10};
            nothrow_exp_t e2{flag_error, 'x'};
            e2 = e1;
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = throw_exp_t{flag_value, 10};
            auto const e2 = nothrow_exp_t::val(10);
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = throw_exp_t{flag_value, 10};
            auto const e2 = nothrow_exp_t::val(9);
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = throw_exp_t{flag_error, 'a'};
            auto const e2 = nothrow_exp_t::val(10);
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = throw_exp_t{flag_value, 10};
            auto const e2 = nothrow_exp_t::err('p');
            REQUIRE(e1 != e2);
        }
        {
            auto const e1 = throw_exp_t{flag_error, 'a'};
            auto const e2 = nothrow_exp_t::err('a');
            REQUIRE(e1 == e2);
        }
        {
            auto const e1 = throw_exp_t{flag_error, 'a'};
            auto const e2 = nothrow_exp_t::err('b');
            REQUIRE(e1 != e2);
        }
    }
    {
        using exp_pointer_t = expected<int *, char const *>;
        {
            int x{1};
            int y{1};
            auto const e1 = exp_pointer_t{flag_value, &x};
            auto const e2 = exp_pointer_t{flag_value, &y};
            REQUIRE(e1 != e2);
        }
        {
            int x{1};
            auto const e1 = exp_pointer_t{flag_value, &x};
            auto const e2 = exp_pointer_t{flag_value, &x};
            REQUIRE(e1 == e2);
        }
        {
            int x{1};
            char c{'t'};
            auto const e1 = exp_pointer_t{flag_value, &x};
            auto const e2 = exp_pointer_t{flag_error, &c};
            REQUIRE(e1 != e2);
        }
    }
    {
        using exp_pointer_t = expected<int &, char const &>;
        {
            int x{1};
            int y{1};
            auto const e1 = exp_pointer_t{flag_value, x};
            auto const e2 = exp_pointer_t{flag_value, y};
            REQUIRE(e1 == e2);
        }
        {
            int x{1};
            auto const e1 = exp_pointer_t{flag_value, x};
            auto const e2 = exp_pointer_t{flag_value, x};
            REQUIRE(e1 == e2);
        }
        {
            int x{1};
            char c{'t'};
            auto const e1 = exp_pointer_t{flag_value, x};
            auto const e2 = exp_pointer_t{flag_error, c};
            REQUIRE(e1 != e2);
        }
    }
}

TEST_CASE("expected", "[value/error getter]") {  // NOLINT
    {
        using exp_t = expected<int, char>;
        {
            auto const e = exp_t::val(10);
            STATIC_REQUIRE(std::is_same_v<decltype(e.value()), int const &>);
            REQUIRE(e.value() == 10);
        }
        {
            auto const e = exp_t::err('x');
            STATIC_REQUIRE(std::is_same_v<decltype(e.error()), char const &>);
            REQUIRE(e.error() == 'x');
        }
        {
            auto const e = exp_t::val(10);
            REQUIRE_THROWS_AS(e.error(), bad_expected_error_access);
            REQUIRE_THROWS_AS(e.error(), bad_expected_access);
        }
        {
            auto const e = exp_t::err('x');
            REQUIRE_THROWS_AS(e.value(), bad_expected_value_access);
            REQUIRE_THROWS_AS(e.value(), bad_expected_access);
        }
    }
    {
        {
            using exp_t = expected<int *, char *>;
            int v = 10;  // NOLINT
            auto const e = exp_t::val(&v);
            STATIC_REQUIRE(std::is_same_v<decltype(e.value()), std::add_lvalue_reference_t<int *const>>);
            REQUIRE(e.value() == &v);
        }
        {
            using exp_t = expected<int const *, char const *>;
            {
                char const c = 'x';
                auto const e = exp_t::err(&c);
                STATIC_REQUIRE(std::is_same_v<decltype(e.error()), std::add_lvalue_reference_t<char const *const>>);
                REQUIRE(e.error() == &c);
            }
            {
                char const c = 'x';
                auto e = exp_t::err(&c);
                STATIC_REQUIRE(std::is_same_v<decltype(e.error()), std::add_lvalue_reference_t<char const *>>);
                REQUIRE(e.error() == &c);
            }
            {
                int const v = 10;
                auto const e = exp_t::val(&v);
                REQUIRE_THROWS_AS(e.error(), bad_expected_error_access);
                REQUIRE_THROWS_AS(e.error(), bad_expected_access);
            }
            {
                char const c = 'x';
                auto const e = exp_t::err(&c);
                REQUIRE_THROWS_AS(e.value(), bad_expected_value_access);
                REQUIRE_THROWS_AS(e.value(), bad_expected_access);
            }
        }
    }
}

TEST_CASE("expected", "[value/error getter - references]") {  // NOLINT
    {
        using exp_t = expected<int &, char &>;
        int v = 10;  // NOLINT
        auto const e = exp_t::val(v);
        STATIC_REQUIRE(std::is_same_v<decltype(e.value()), int &>);
        REQUIRE(e.value() == v);
    }
    {
        using exp_t = expected<int &, char &>;
        int v = 10;  // NOLINT
        auto e = exp_t::val(v);
        STATIC_REQUIRE(std::is_same_v<decltype(e.value()), int &>);
        REQUIRE(e.value() == v);
    }
    {
        using exp_t = expected<int const &, char const &>;
        {
            char const c = 'x';
            auto const e = exp_t::err(c);
            STATIC_REQUIRE(std::is_same_v<decltype(e.error()), char const &>);
            REQUIRE(e.error() == c);
        }
        {
            char const c = 'x';
            auto e = exp_t::err(c);
            STATIC_REQUIRE(std::is_same_v<decltype(e.error()), char const &>);
            REQUIRE(e.error() == c);
        }
        {
            char c = 'x';
            auto e = exp_t::err(c);
            STATIC_REQUIRE(std::is_same_v<decltype(e.error()), char const &>);
            REQUIRE(e.error() == c);
        }
        {
            int const v = 10;
            auto const e = exp_t::val(v);
            REQUIRE_THROWS_AS(e.error(), bad_expected_error_access);
            REQUIRE_THROWS_AS(e.error(), bad_expected_access);
        }
        {
            char const c = 'x';
            auto const e = exp_t::err(c);
            REQUIRE_THROWS_AS(e.value(), bad_expected_value_access);
            REQUIRE_THROWS_AS(e.value(), bad_expected_access);
        }
    }
}

TEST_CASE("expected", "[value/error getter - rvalues]") {  // NOLINT
    {
        using exp_t = expected<int, char>;

        STATIC_REQUIRE(std::is_same_v<decltype(exp_t::val(10).value()), int &&>);
        STATIC_REQUIRE(std::is_same_v<decltype(exp_t::err('x').error()), char &&>);
        {
            auto const e = exp_t::err('x');
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).error()), char const &&>);
        }
        {
            auto const e = exp_t::val(10);
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).value()), int const &&>);
        }
    }
    {
        using exp_t = expected<int *, char const *>;
        {
            int x{10};  // NOLINT
            STATIC_REQUIRE(std::is_same_v<decltype(exp_t::val(&x).value()), std::add_rvalue_reference_t<int *>>);
        }
        {
            char x{'x'};  // NOLINT
            STATIC_REQUIRE(std::is_same_v<decltype(exp_t::err(&x).error()), std::add_rvalue_reference_t<char const *>>);
        }
        {
            int x{10};  // NOLINT
            auto const e = exp_t::val(&x);
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).value()), std::add_rvalue_reference_t<int *const>>);
        }
        {
            char x{'x'};  // NOLINT
            auto const e = exp_t::err(&x);
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).error()), std::add_rvalue_reference_t<char const *const>>);
        }
    }
    {
        using exp_t = expected<int &, char const &>;
        {
            int x{10};  // NOLINT
            STATIC_REQUIRE(std::is_same_v<decltype(exp_t::val(x).value()), std::add_lvalue_reference_t<int>>);
        }
        {
            char x{'x'};  // NOLINT
            STATIC_REQUIRE(std::is_same_v<decltype(exp_t::err(x).error()), std::add_lvalue_reference_t<char const>>);
        }
        {
            int x{10};  // NOLINT
            auto const e = exp_t::val(x);
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).value()), std::add_lvalue_reference_t<int>>);
        }
        {
            char x{'x'};  // NOLINT
            auto const e = exp_t::err(x);
            STATIC_REQUIRE(std::is_same_v<decltype(std::move(e).error()), std::add_lvalue_reference_t<char const>>);
        }
    }
}

TEST_CASE("expected", "[into-optional-conversions]") {
    using type_t = expected<move_detector, char>;
    auto v = type_t::val(move_detector{});
    auto opt = v.into_optional();
    STATIC_REQUIRE(std::is_same_v<decltype(opt), std::optional<move_detector>>);
    REQUIRE(v.value().moved);
    REQUIRE(opt.has_value());
}

TEST_CASE("expected", "[as-optional-conversions]") {  // NOLINT
    {
        using type_t = expected<move_detector, char>;
        {
            auto v = type_t::val(move_detector{});
            auto opt = v.as_optional();
            STATIC_REQUIRE(std::is_same_v<decltype(opt), std::optional<move_detector>>);
            REQUIRE_FALSE(v.value().moved);
            REQUIRE(opt.has_value());
        }
        {
            auto v = type_t::val(move_detector{});
            auto opt = std::move(v).as_optional();
            STATIC_REQUIRE(std::is_same_v<decltype(opt), std::optional<move_detector>>);
            REQUIRE(v.value().moved);
            REQUIRE(opt.has_value());
        }
    }
    {
        using type_t = expected<int &, char>;
        {
            int x = 10;
            auto v = type_t::val(x);
            auto opt = v.as_optional();
            STATIC_REQUIRE(std::is_same_v<decltype(opt), optional_ref<int>>);
            REQUIRE(opt.has_value());
            REQUIRE(opt.value() == x);
        }
        {
            int x = 10;
            auto const v = type_t::val(x);
            auto opt = v.as_optional();
            STATIC_REQUIRE(std::is_same_v<decltype(opt), optional_ref<int>>);
            REQUIRE(opt.has_value());
            REQUIRE(opt.value() == x);
        }
    }
    {
        using type_t = expected<int, char>;
        {
            auto v = type_t::val(10);
            auto opt = v.as_optional();
            STATIC_REQUIRE(std::is_same_v<decltype(opt), std::optional<int>>);
            REQUIRE(opt.has_value());
            REQUIRE(opt.value() == 10);
        }
    }
}

// TODO: special operators: copy/move/copy-assign/move-assign