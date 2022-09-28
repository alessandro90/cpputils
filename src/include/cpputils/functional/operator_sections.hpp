#ifndef CPPUTILS_OPERATOR_SECTIONS_HPP
#define CPPUTILS_OPERATOR_SECTIONS_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>


// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FWD(x) std::forward<decltype(x)>(x)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_PARTIAL_FUNCTION(op)                                                \
    [[nodiscard]] friend constexpr auto operator op(wildcard, auto &&rhs) noexcept { \
        return [rhs_ = FWD(rhs)](auto &&lhs) requires requires { lhs op FWD(rhs); }  \
        { return FWD(lhs) op rhs_; };                                                \
    }                                                                                \
                                                                                     \
    [[nodiscard]] friend constexpr auto operator op(auto &&lhs, wildcard) noexcept { \
        return [lhs_ = FWD(lhs)](auto &&rhs) requires requires { lhs op FWD(rhs); }  \
        { return lhs_ op FWD(rhs); };                                                \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_BINARY_FUNCTION(op, handler)                           \
    [[nodiscard]] constexpr auto operator op(wildcard) const noexcept { \
        return handler<>{}; /*NOLINT*/                                  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WILDCARD_UNARY_FUNCTION(op, handler)                    \
    [[nodiscard]] constexpr auto operator op() const noexcept { \
        return handler<>{}; /*NOLINT*/                          \
    }

namespace cpputils {
namespace detail {
    struct wildcard {
        WILDCARD_BINARY_FUNCTION(*, std::multiplies)
        WILDCARD_BINARY_FUNCTION(/, std::divides)
        WILDCARD_BINARY_FUNCTION(+, std::plus)
        WILDCARD_BINARY_FUNCTION(-, std::minus)
        WILDCARD_BINARY_FUNCTION(%, std::modulus)
        WILDCARD_UNARY_FUNCTION(-, std::negate)

        WILDCARD_PARTIAL_FUNCTION(*)
        WILDCARD_PARTIAL_FUNCTION(/)
        WILDCARD_PARTIAL_FUNCTION(+)
        WILDCARD_PARTIAL_FUNCTION(-)
        WILDCARD_PARTIAL_FUNCTION(%)

        WILDCARD_BINARY_FUNCTION(==, std::equal_to)
        WILDCARD_BINARY_FUNCTION(!=, std::not_equal_to)
        WILDCARD_BINARY_FUNCTION(>, std::greater)
        WILDCARD_BINARY_FUNCTION(<, std::less)
        WILDCARD_BINARY_FUNCTION(>=, std::greater_equal)
        WILDCARD_BINARY_FUNCTION(<=, std::less_equal)
        WILDCARD_UNARY_FUNCTION(!, std::logical_not)

        WILDCARD_PARTIAL_FUNCTION(==)
        WILDCARD_PARTIAL_FUNCTION(!=)
        WILDCARD_PARTIAL_FUNCTION(>)
        WILDCARD_PARTIAL_FUNCTION(<)
        WILDCARD_PARTIAL_FUNCTION(>=)
        WILDCARD_PARTIAL_FUNCTION(<=)

        WILDCARD_BINARY_FUNCTION(&&, std::logical_and)
        WILDCARD_BINARY_FUNCTION(||, std::logical_or)

        WILDCARD_PARTIAL_FUNCTION(&&)
        WILDCARD_PARTIAL_FUNCTION(||)

        WILDCARD_BINARY_FUNCTION(&, std::bit_and)
        WILDCARD_BINARY_FUNCTION(|, std::bit_or)
        WILDCARD_BINARY_FUNCTION(^, std::bit_xor)
        WILDCARD_UNARY_FUNCTION(~, std::bit_not)

        WILDCARD_PARTIAL_FUNCTION(&)
        WILDCARD_PARTIAL_FUNCTION(|)
        WILDCARD_PARTIAL_FUNCTION(^)

        constexpr auto fn(auto &&f) const noexcept {
            return [f_ = FWD(f)](auto &&obj) -> decltype(auto) requires std::invocable<std::remove_reference_t<decltype(f)>, decltype(obj)>
            {
                return std::invoke(f_, FWD(obj));
            };
        }
    };
}  // namespace detail

inline constexpr auto _ = detail::wildcard{};

}  // namespace cpputils

#undef FWD
#undef WILDCARD_PARTIAL_FUNCTION
#undef WILDCARD_BINARY_FUNCTION
#undef WILDCARD_UNARY_FUNCTION

#endif
