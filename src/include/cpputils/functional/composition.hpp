#ifndef CPPUTILS_COMPOSITION_HPP
#define CPPUTILS_COMPOSITION_HPP

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>


// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {
template <typename>
struct composer_t;

namespace detail {
    template <typename F, typename G, typename... Ts>
    concept chainable = std::invocable<G, Ts...> && std::invocable<F, std::invoke_result_t<G, Ts...>>;

    constexpr auto chain_invoke(auto &&f, auto &&g, auto &&...args) requires chainable<decltype(FWD(f)), decltype(FWD(g)), decltype(FWD(args))...>
    {
        return std::invoke(FWD(f), std::invoke(FWD(g), FWD(args)...));
    }

    template <typename F>
    [[nodiscard]] constexpr auto make_composer(F &&f) {
        return composer_t<std::remove_cvref_t<F>>{FWD(f)};
    }
}  // namespace detail
template <typename F = void>
struct composer_t {
    F f;

    [[nodiscard]] constexpr auto operator<<(auto &&g) const && {
        return detail::make_composer([f_ = std::move(f), g_ = FWD(g)](auto &&...xs) {
            return detail::chain_invoke(f_, g_, FWD(xs)...);
        });
    }

    [[nodiscard]] constexpr auto operator>>(auto &&g) const && {
        return detail::make_composer([f_ = std::move(f), g_ = FWD(g)](auto &&...xs) {
            return detail::chain_invoke(g_, f_, FWD(xs)...);
        });
    }

    constexpr decltype(auto) operator()(auto &&...args) const
        requires std::invocable<decltype(f), decltype(args)...>
    {
        return std::invoke(f, FWD(args)...);
    }

    constexpr decltype(auto) operator|(auto &&arg) const {
        return this->operator()(FWD(arg));
    }
};

template <>
struct composer_t<void> {
    [[nodiscard]] constexpr auto operator<<(auto &&g) const {
        return detail::make_composer(
            [g_ = FWD(g)](auto &&...xs) requires std::invocable<decltype(g), decltype(xs)...>  //
            {
                return std::invoke(g_, FWD(xs)...);
            });
    }

    [[nodiscard]] constexpr auto operator>>(auto &&g) const {
        return *this << FWD(g);
    }
};

inline constexpr auto compose = composer_t<>{};

[[nodiscard]] constexpr auto compose_r(auto &&...fs) {
    static_assert(sizeof...(fs) > 0);
    return (compose << ... << FWD(fs));
}

[[nodiscard]] constexpr auto compose_l(auto &&...fs) {
    static_assert(sizeof...(fs) > 0);
    return (compose >> ... >> FWD(fs));
}

#undef FWD
}  // namespace cpputils

#endif