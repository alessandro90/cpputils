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
namespace detail {
    // template <typename... Fs>
    // constexpr decltype(auto) compose_impl(std::tuple<Fs...> fs, auto... args) {
    //     if constexpr (sizeof...(Fs) == 1) {
    //         return std::get<0>(fs)(args...);
    //     } else {
    //         return compose_impl(
    //             [=]<std::size_t... Ns>(std::index_sequence<Ns...>) {
    //                 return std::tuple{std::get<Ns>(fs)...};
    //             }(std::make_index_sequence<sizeof...(Fs) - 1>{}),
    //             std::get<sizeof...(Fs) - 1>(fs)(args...));
    //     }
    // }

    template <typename F, typename G, typename... Ts>
    concept chainable = std::invocable<G, Ts...> && std::invocable<F, std::invoke_result_t<G, Ts...>>;

    constexpr auto chain_invoke(auto &&f, auto &&g, auto &&...args) requires chainable<decltype(FWD(f)), decltype(FWD(g)), decltype(FWD(args))...> {
        return std::invoke(FWD(f), std::invoke(FWD(g), FWD(args)...));
    }

    template <typename F = void>
    struct composer {
        F f;

        constexpr auto operator*(auto &&g) const && {
            return make_composer([f_ = std::move(f), g_ = FWD(g)](auto &&...xs) {
                return chain_invoke(f_, g_, FWD(xs)...);
            });
        }

        constexpr auto operator/(auto &&g) const && {
            return make_composer([f_ = std::move(f), g_ = FWD(g)](auto &&...xs) {
                return chain_invoke(g_, f_, FWD(xs)...);
            });
        }
    };

    template <>
    struct composer<void> {
        constexpr auto operator*(auto &&g) const {
            return make_composer([g_ = FWD(g)](auto &&...xs) requires std::invocable<decltype(g), decltype(FWD(xs))...> {
                return std::invoke(g_, FWD(xs)...);
            });
        }

        constexpr auto operator/(auto &&g) const {
            return *this * FWD(g);
        }
    };

    template <typename F>
    constexpr auto make_composer(F &&f) {
        return composer<std::remove_cvref_t<F>>{FWD(f)};
    }
}  // namespace detail

// constexpr auto compose(auto... fs) requires(sizeof...(fs) > 0) {
//     return [fs...](auto... args) -> decltype(auto) {
//         return detail::compose_impl(std::tuple{fs...}, args...);
//     };
// }

constexpr auto compose_right(auto &&...fs) {
    static_assert(sizeof...(fs) > 0);
    return (detail::composer<>{} * ... * FWD(fs)).f;
}

consteval auto consteval_compose_right(auto &&...fs) {
    return compose_right(FWD(fs)...);
}

constexpr auto compose_left(auto &&...fs) {
    static_assert(sizeof...(fs) > 0);
    return (detail::composer<>{} / ... / FWD(fs)).f;
}

consteval auto consteval_compose_left(auto &&...fs) {
    return compose_left(FWD(fs)...);
}

#undef FWD
}  // namespace cpputils

#endif