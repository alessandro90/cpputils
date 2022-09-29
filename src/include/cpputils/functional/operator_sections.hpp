#ifndef CPPUTILS_OPERATOR_SECTIONS_HPP
#define CPPUTILS_OPERATOR_SECTIONS_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace cpputils::detail {
template <typename Func>
struct wildcard_callable;

struct wildcard;

template <typename>
struct is_wildcard_callable : std::false_type {};

template <typename T>
struct is_wildcard_callable<wildcard_callable<T>> : std::true_type {};

template <typename>
struct is_wildcard : std::false_type {};

template <>
struct is_wildcard<wildcard> : std::true_type {};

// clang-format off
template <typename T>
concept not_wildcard = !is_wildcard<T>::value && !is_wildcard_callable<T>::value;
// clang-format on
}  // namespace cpputils::detail

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FWD(x) std::forward<decltype(x)>(x)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_PARTIAL_FUNCTION(op)                                                      \
    [[nodiscard]] constexpr auto operator op(wildcard, not_wildcard auto &&rhs) noexcept { \
        return [rhs_ = FWD(rhs)](auto &&lhs) requires requires { lhs op FWD(rhs); }        \
        { return FWD(lhs) op rhs_; };                                                      \
    }                                                                                      \
                                                                                           \
    [[nodiscard]] constexpr auto operator op(not_wildcard auto &&lhs, wildcard) noexcept { \
        return [lhs_ = FWD(lhs)](auto &&rhs) requires requires { lhs op FWD(rhs); }        \
        { return lhs_ op FWD(rhs); };                                                      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_BINARY_FUNCTION(op, handler)                           \
    [[nodiscard]] constexpr auto operator op(wildcard) const noexcept { \
        return handler; /*NOLINT*/                                      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_UNARY_FUNCTION(op, handler)                    \
    [[nodiscard]] constexpr auto operator op() const noexcept { \
        return handler; /*NOLINT*/                              \
    }

// -----------------------------------------------------------------
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_CALLABLE_PARTIAL_FUNCTION(op)                                                                  \
    template <typename F>                                                                                       \
    [[nodiscard]] constexpr auto operator op(wildcard_callable<F> const &w, not_wildcard auto &&rhs) noexcept { \
        return [w, rhs_ = FWD(rhs)](auto &&lhs) requires requires { std::invoke(w.f, FWD(lhs)) op FWD(rhs); }   \
        { return std::invoke(w.f, FWD(lhs)) op rhs_; };                                                         \
    }                                                                                                           \
                                                                                                                \
    template <typename F>                                                                                       \
    [[nodiscard]] constexpr auto operator op(wildcard_callable<F> const &w, wildcard) noexcept {                \
        return [w](auto &&lhs, auto &&rhs) {                                                                    \
            return std::invoke(w.f, FWD(lhs)) op FWD(rhs);                                                      \
        };                                                                                                      \
    }                                                                                                           \
    template <typename F>                                                                                       \
    [[nodiscard]] constexpr auto operator op(not_wildcard auto &&lhs, wildcard_callable<F> const &w) noexcept { \
        return [w, lhs_ = FWD(lhs)](auto &&rhs) requires requires { lhs op std::invoke(w.f, FWD(rhs)); }        \
        { return lhs_ op std::invoke(w.f, FWD(rhs)); };                                                         \
    }                                                                                                           \
                                                                                                                \
    template <typename F>                                                                                       \
    [[nodiscard]] constexpr auto operator op(wildcard, wildcard_callable<F> const &w) noexcept {                \
        return [w](auto &&lhs, auto &&rhs) {                                                                    \
            return FWD(lhs) op std::invoke(w.f, FWD(rhs));                                                      \
        };                                                                                                      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_CALLABLE_BINARY_FUNCTION(op, handler)                                       \
    template <typename F>                                                                    \
    [[nodiscard]] constexpr auto operator op(wildcard_callable<F> const &w) const noexcept { \
        return [*this, w](auto const &lhs, auto const &rhs) {                                \
            return std::invoke(handler, std::invoke(f, lhs), std::invoke(w.f, rhs));         \
        };                                                                                   \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_CALLABLE_UNARY_FUNCTION(op, handler)           \
    [[nodiscard]] constexpr auto operator op() const noexcept { \
        return [*this](auto const &v) {                         \
            return std::invoke(handler, std::invoke(f, v));     \
        };                                                      \
    }

namespace cpputils {
namespace detail {
    template <typename Func>
    struct wildcard_callable {
        Func f;

        WILDCARD_CALLABLE_BINARY_FUNCTION(*, std::multiplies<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(/, std::divides<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(+, std::plus<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(-, std::minus<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(%, std::modulus<>{})
        WILDCARD_CALLABLE_UNARY_FUNCTION(-, std::negate<>{})


        WILDCARD_CALLABLE_BINARY_FUNCTION(==, std::equal_to<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(!=, std::not_equal_to<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(>, std::greater<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(<, std::less<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(>=, std::greater_equal<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(<=, std::less_equal<>{})
        WILDCARD_CALLABLE_UNARY_FUNCTION(!, std::logical_not<>{})

        WILDCARD_CALLABLE_BINARY_FUNCTION(&&, std::logical_and<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(||, std::logical_or<>{})


        WILDCARD_CALLABLE_BINARY_FUNCTION(&, std::bit_and<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(|, std::bit_or<>{})
        WILDCARD_CALLABLE_BINARY_FUNCTION(^, std::bit_xor<>{})
        WILDCARD_CALLABLE_UNARY_FUNCTION(~, std::bit_not<>{})


        constexpr decltype(auto) operator()(auto &&...args) const {
            return std::invoke(f, FWD(args)...);
        }
    };

    struct wildcard {
        WILDCARD_BINARY_FUNCTION(*, std::multiplies<>{})
        WILDCARD_BINARY_FUNCTION(/, std::divides<>{})
        WILDCARD_BINARY_FUNCTION(+, std::plus<>{})
        WILDCARD_BINARY_FUNCTION(-, std::minus<>{})
        WILDCARD_BINARY_FUNCTION(%, std::modulus<>{})
        WILDCARD_UNARY_FUNCTION(-, std::negate<>{})


        WILDCARD_BINARY_FUNCTION(==, std::equal_to<>{})
        WILDCARD_BINARY_FUNCTION(!=, std::not_equal_to<>{})
        WILDCARD_BINARY_FUNCTION(>, std::greater<>{})
        WILDCARD_BINARY_FUNCTION(<, std::less<>{})
        WILDCARD_BINARY_FUNCTION(>=, std::greater_equal<>{})
        WILDCARD_BINARY_FUNCTION(<=, std::less_equal<>{})
        WILDCARD_UNARY_FUNCTION(!, std::logical_not<>{})


        WILDCARD_BINARY_FUNCTION(&&, std::logical_and<>{})
        WILDCARD_BINARY_FUNCTION(||, std::logical_or<>{})


        WILDCARD_BINARY_FUNCTION(&, std::bit_and<>{})
        WILDCARD_BINARY_FUNCTION(|, std::bit_or<>{})
        WILDCARD_BINARY_FUNCTION(^, std::bit_xor<>{})
        WILDCARD_UNARY_FUNCTION(~, std::bit_not<>{})

        // clang-format off
        constexpr auto fn(auto &&f) const noexcept {
            return wildcard_callable{
                [f_ = FWD(f)](auto &&obj) -> decltype(auto) requires std::invocable<std::remove_reference_t<decltype(f)>, decltype(obj)> {
                    return std::invoke(f_, FWD(obj));
                }
            };
    // clang-format on
}  // namespace detail
};  // namespace cpputils

WILDCARD_PARTIAL_FUNCTION(*)
WILDCARD_PARTIAL_FUNCTION(/)
WILDCARD_PARTIAL_FUNCTION(+)
WILDCARD_PARTIAL_FUNCTION(-)
WILDCARD_PARTIAL_FUNCTION(%)

WILDCARD_PARTIAL_FUNCTION(==)
WILDCARD_PARTIAL_FUNCTION(!=)
WILDCARD_PARTIAL_FUNCTION(>)
WILDCARD_PARTIAL_FUNCTION(<)
WILDCARD_PARTIAL_FUNCTION(>=)
WILDCARD_PARTIAL_FUNCTION(<=)

WILDCARD_PARTIAL_FUNCTION(&&)
WILDCARD_PARTIAL_FUNCTION(||)

WILDCARD_PARTIAL_FUNCTION(&)
WILDCARD_PARTIAL_FUNCTION(|)
WILDCARD_PARTIAL_FUNCTION(^)

WILDCARD_CALLABLE_PARTIAL_FUNCTION(*)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(/)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(+)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(-)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(%)

WILDCARD_CALLABLE_PARTIAL_FUNCTION(==)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(!=)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(>)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(<)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(>=)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(<=)


WILDCARD_CALLABLE_PARTIAL_FUNCTION(&&)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(||)

WILDCARD_CALLABLE_PARTIAL_FUNCTION(&)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(|)
WILDCARD_CALLABLE_PARTIAL_FUNCTION(^)
}  // namespace detail

inline constexpr auto _ = detail::wildcard{};

}  // namespace cpputils

#undef FWD
#undef WILDCARD_PARTIAL_FUNCTION
#undef WILDCARD_BINARY_FUNCTION
#undef WILDCARD_UNARY_FUNCTION
#undef WILDCARD_CALLABLE_PARTIAL_FUNCTION
#undef WILDCARD_CALLABLE_BINARY_FUNCTION
#undef WILDCARD_CALLABLE_UNARY_FUNCTION
#endif
