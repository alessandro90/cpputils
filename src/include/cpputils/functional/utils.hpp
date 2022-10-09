#ifndef CPPUTILS_FUNCTIONAL_UTILS_HPP
#define CPPUTILS_FUNCTIONAL_UTILS_HPP

#include "../meta/traits.hpp"
#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>


// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {
[[nodiscard]] constexpr auto flip(auto f) noexcept {
    return [f](auto &&a, auto &&b)
        requires std::invocable<decltype(f), decltype(a), decltype(b)>
    {
        return std::invoke(f, FWD(b), FWD(a));
    };
}

[[nodiscard]] constexpr auto partial_l(auto f, auto &&...args) noexcept {
    return [f, ... args_ = FWD(args)](auto &&...more_args)
        requires std::invocable<decltype(f), decltype(args)..., decltype(more_args)...>
    {
        return std::invoke(f, args_..., FWD(more_args)...);
    };
}

[[nodiscard]] constexpr auto partial_r(auto f, auto &&...args) noexcept {
    return [f, ... args_ = FWD(args)](auto &&...more_args)
        requires std::invocable<decltype(f), decltype(more_args)..., decltype(args)...>
    {
        return std::invoke(f, FWD(more_args)..., args_...);
    };
}

template <std::ranges::range Range, invocable_r<std::ranges::range_value_t<Range>> Generator>
struct range_maker {
    [[nodiscard]] constexpr auto operator()(Generator g, std::size_t num_items) const
        requires std::default_initializable<Range>
                 && (
                     requires (Range r, Generator g_) { r.push_back(g_()); }
                     || requires (Range r, Generator g_) { r.insert(g_()); })
    {
        Range rng{};
        if constexpr (std::ranges::sized_range<Range> && requires { rng.reserve(); }) {
            rng.reserve(num_items);
        }
        if constexpr (requires { rng.push_back(g()); }) {
            std::ranges::generate_n(std::back_inserter(rng),
                                    static_cast<std::iter_difference_t<decltype(std::back_inserter(rng))>>(num_items),
                                    g);
        } else {
            for (std::size_t i = 0; i < num_items; ++i) {
                rng.insert(g());
            }
        }

        return rng;
    }
};

namespace detail {
    template <typename T>
    struct is_cstyle_array : std::false_type {};

    template <typename T, std::size_t N>
    struct is_cstyle_array<T[N]> : std::true_type {};  // NOLINT
}  // namespace detail

template <typename T, std::size_t N, invocable_r<T> Generator>
struct range_maker<std::array<T, N>, Generator> {
    [[nodiscard]] constexpr auto operator()(Generator g) const {
        return generator_function(std::move(g), std::make_index_sequence<N>{});
    }

private:
    template <auto... Is>
    [[nodiscard]] static constexpr auto generator_function(Generator g, std::index_sequence<Is...>) {
        return std::array<T, N>{[&](auto) { return g(); }(Is)...};
    }
};

template <typename Range>
inline constexpr auto make_range = []<typename Generator, typename... Args>(Generator g, Args... args)
    requires std::invocable<range_maker<Range, Generator>, Generator, Args...>
{
    static_assert(!detail::is_cstyle_array<Range>::value, "C-style arrays cannot be returned from functions, use std::array");
    auto const factory = range_maker<Range, Generator>{};
    return factory(std::move(g), args...);
};

}  // namespace cpputils

#undef FWD

#endif
