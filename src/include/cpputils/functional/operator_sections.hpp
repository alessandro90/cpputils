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

template <typename Obj, typename F>
concept object_method_call = std::is_class_v<std::decay_t<Obj>> && std::is_member_pointer_v<F>;
}  // namespace cpputils::detail

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FWD(x) std::forward<decltype(x)>(x)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_PARTIAL_FUNCTION(op)                                                           \
    [[nodiscard]] constexpr auto operator op(wildcard, not_wildcard auto const &rhs) noexcept { \
        return [rhs](auto const &lhs) requires requires { lhs op rhs; }                         \
        { return lhs op rhs; };                                                                 \
    }                                                                                           \
                                                                                                \
    [[nodiscard]] constexpr auto operator op(not_wildcard auto const &lhs, wildcard) noexcept { \
        return [lhs](auto const &rhs) requires requires { lhs op rhs; }                         \
        { return lhs op rhs; };                                                                 \
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
#define WILDCARD_CALLABLE_PARTIAL_FUNCTION(op)                                                                             \
    template <typename F>                                                                                                  \
    [[nodiscard]] constexpr auto operator op(wildcard_callable<F> const &w, not_wildcard auto const &rhs) noexcept {       \
        return [w, rhs](auto const &lhs) -> decltype(auto) requires requires { std::invoke(w.f, lhs) op rhs; }             \
        {                                                                                                                  \
            return std::invoke(w.f, lhs) op rhs;                                                                           \
        };                                                                                                                 \
    }                                                                                                                      \
                                                                                                                           \
    template <typename F>                                                                                                  \
    [[nodiscard]] constexpr auto operator op(wildcard_callable<F> const &w, wildcard) noexcept {                           \
        return [w](auto const &lhs, auto const &rhs) -> decltype(auto) requires requires { std::invoke(w.f, lhs) op rhs; } \
        {                                                                                                                  \
            return std::invoke(w.f, lhs) op rhs;                                                                           \
        };                                                                                                                 \
    }                                                                                                                      \
    template <typename F>                                                                                                  \
    [[nodiscard]] constexpr auto operator op(not_wildcard auto const &lhs, wildcard_callable<F> const &w) noexcept {       \
        return [w, lhs](auto const &rhs) -> decltype(auto) requires requires { lhs op std::invoke(w.f, rhs); }             \
        { return lhs op std::invoke(w.f, rhs); };                                                                          \
    }                                                                                                                      \
                                                                                                                           \
    template <typename F>                                                                                                  \
    [[nodiscard]] constexpr auto operator op(wildcard, wildcard_callable<F> const &w) noexcept {                           \
        return [w](auto const &lhs, auto const &rhs) -> decltype(auto) requires requires { lhs op std::invoke(w.f, rhs); } \
        {                                                                                                                  \
            return lhs op std::invoke(w.f, rhs);                                                                           \
        };                                                                                                                 \
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

// Allow binary operations between different types
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_BINARY_OPERATOR(op_name, op)                                                                                  \
    inline constexpr auto op_name = [](auto const &lhs, auto const &rhs) -> decltype(auto) requires requires { lhs op rhs; } \
    {                                                                                                                        \
        return lhs op rhs;                                                                                                   \
    };

namespace cpputils {

DEFINE_BINARY_OPERATOR(multiplies, *)
DEFINE_BINARY_OPERATOR(divides, /)
DEFINE_BINARY_OPERATOR(plus, +)
DEFINE_BINARY_OPERATOR(minus, -)
DEFINE_BINARY_OPERATOR(modulus, %)
DEFINE_BINARY_OPERATOR(equal_to, ==)
DEFINE_BINARY_OPERATOR(not_equal_to, !=)
DEFINE_BINARY_OPERATOR(greater, >)
DEFINE_BINARY_OPERATOR(less, <)
DEFINE_BINARY_OPERATOR(greater_equal, >=)
DEFINE_BINARY_OPERATOR(less_equal, <=)
DEFINE_BINARY_OPERATOR(logical_and, &&)
DEFINE_BINARY_OPERATOR(logical_or, ||)
DEFINE_BINARY_OPERATOR(bit_and, &)
DEFINE_BINARY_OPERATOR(bit_or, |)
DEFINE_BINARY_OPERATOR(bit_xor, ^)

namespace detail {
    template <typename Func>
    struct wildcard_callable {
        Func f;

        WILDCARD_CALLABLE_BINARY_FUNCTION(*, multiplies)
        WILDCARD_CALLABLE_BINARY_FUNCTION(/, divides)
        WILDCARD_CALLABLE_BINARY_FUNCTION(+, plus)
        WILDCARD_CALLABLE_BINARY_FUNCTION(-, minus)
        WILDCARD_CALLABLE_BINARY_FUNCTION(%, modulus)
        WILDCARD_CALLABLE_UNARY_FUNCTION(-, std::negate<>{})


        WILDCARD_CALLABLE_BINARY_FUNCTION(==, equal_to)
        WILDCARD_CALLABLE_BINARY_FUNCTION(!=, not_equal_to)
        WILDCARD_CALLABLE_BINARY_FUNCTION(>, greater)
        WILDCARD_CALLABLE_BINARY_FUNCTION(<, less)
        WILDCARD_CALLABLE_BINARY_FUNCTION(>=, greater_equal)
        WILDCARD_CALLABLE_BINARY_FUNCTION(<=, less_equal)
        WILDCARD_CALLABLE_UNARY_FUNCTION(!, std::logical_not<>{})

        WILDCARD_CALLABLE_BINARY_FUNCTION(&&, logical_and)
        WILDCARD_CALLABLE_BINARY_FUNCTION(||, logical_or)


        WILDCARD_CALLABLE_BINARY_FUNCTION(&, bit_and)
        WILDCARD_CALLABLE_BINARY_FUNCTION(|, bit_or)
        WILDCARD_CALLABLE_BINARY_FUNCTION(^, bit_xor)
        WILDCARD_CALLABLE_UNARY_FUNCTION(~, std::bit_not<>{})


        constexpr decltype(auto) operator()(auto &&...args) const requires std::invocable<decltype(f), decltype(args)...>
        {
            return std::invoke(f, FWD(args)...);
        }

        // clang-format off
        [[nodiscard]] constexpr auto fn(auto &&g) const noexcept {
            return make([*this, g_ = FWD(g)](auto const &obj) requires requires { std::invoke(g, std::invoke(f, obj)); } {
                return std::invoke(g_, std::invoke(f, obj));
            });
        }
        // clang-format on

    private:
        static constexpr auto make(auto &&g) noexcept {
            return wildcard_callable<std::remove_cvref_t<decltype(g)>>{FWD(g)};
        }
    };

    struct wildcard {
        WILDCARD_BINARY_FUNCTION(*, multiplies)
        WILDCARD_BINARY_FUNCTION(/, divides)
        WILDCARD_BINARY_FUNCTION(+, plus)
        WILDCARD_BINARY_FUNCTION(-, minus)
        WILDCARD_BINARY_FUNCTION(%, modulus)
        WILDCARD_UNARY_FUNCTION(-, std::negate<>{})


        WILDCARD_BINARY_FUNCTION(==, equal_to)
        WILDCARD_BINARY_FUNCTION(!=, not_equal_to)
        WILDCARD_BINARY_FUNCTION(>, greater)
        WILDCARD_BINARY_FUNCTION(<, less)
        WILDCARD_BINARY_FUNCTION(>=, greater_equal)
        WILDCARD_BINARY_FUNCTION(<=, less_equal)
        WILDCARD_UNARY_FUNCTION(!, std::logical_not<>{})


        WILDCARD_BINARY_FUNCTION(&&, logical_and)
        WILDCARD_BINARY_FUNCTION(||, logical_or)


        WILDCARD_BINARY_FUNCTION(&, bit_and)
        WILDCARD_BINARY_FUNCTION(|, bit_or)
        WILDCARD_BINARY_FUNCTION(^, bit_xor)
        WILDCARD_UNARY_FUNCTION(~, std::bit_not<>{})

        // clang-format off
        constexpr auto fn(auto &&f) const noexcept {
            return wildcard_callable{
                [f_ = FWD(f)](auto const &obj) -> decltype(auto) requires std::invocable<std::remove_cvref_t<decltype(f)>, decltype(obj)> {
                    return std::invoke(f_, obj);
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

#ifdef CPPULTILS_ENABLE_CALL_MACROS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL(f)                                                                                                                                                                                                     \
    [](auto const &obj) -> decltype(auto) requires cpputils::detail::object_method_call<decltype(obj), decltype(&std::decay_t<decltype(obj)>::f)> && requires { std::invoke(std::decay_t<decltype(obj)>::f, obj); } \
    {                                                                                                                                                                                                               \
        return std::invoke(std::decay_t<decltype(obj)>::f, obj);                                                                                                                                                    \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_C(f, ...)                                                                                                                                                                                                            \
    [=](auto const &obj) -> decltype(auto) requires cpputils::detail::object_method_call<decltype(obj), decltype(&std::decay_t<decltype(obj)>::f)> && requires { std::invoke(std::decay_t<decltype(obj)>::f, obj, __VA_ARGS__); } \
    {                                                                                                                                                                                                                             \
        return std::invoke(std::decay_t<decltype(obj)>::f, obj, __VA_ARGS__);                                                                                                                                                     \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_R(f, ...)                                                                                                                                                                                                            \
    [&](auto const &obj) -> decltype(auto) requires cpputils::detail::object_method_call<decltype(obj), decltype(&std::decay_t<decltype(obj)>::f)> && requires { std::invoke(std::decay_t<decltype(obj)>::f, obj, __VA_ARGS__); } \
    {                                                                                                                                                                                                                             \
        return std::invoke(std::decay_t<decltype(obj)>::f, obj, __VA_ARGS__);                                                                                                                                                     \
    }
#endif

inline constexpr auto _ = detail::wildcard{};

}  // namespace cpputils

#undef FWD
#undef WILDCARD_PARTIAL_FUNCTION
#undef WILDCARD_BINARY_FUNCTION
#undef WILDCARD_UNARY_FUNCTION
#undef WILDCARD_CALLABLE_PARTIAL_FUNCTION
#undef WILDCARD_CALLABLE_BINARY_FUNCTION
#undef WILDCARD_CALLABLE_UNARY_FUNCTION
#undef DEFINE_BINARY_OPERATOR
#endif
