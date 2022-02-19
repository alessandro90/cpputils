#ifndef CPPUTILS_ADAPTORS_RANGE_HPP
#define CPPUTILS_ADAPTORS_RANGE_HPP

// Adapted from http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2387r0.html

#include <concepts>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>


// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils::detail::adaptors {


template <typename Callable>
struct range_adaptor_closure;

template <typename Callable>
struct range_adaptor {
    Callable callable;

    constexpr auto operator()(auto &&...args) const {
        if constexpr (std::is_invocable_v<Callable, decltype(args)...>) {
            return std::invoke(callable, FWD(args)...);
        } else {
            return range_adaptor_closure{
                [... args_ = FWD(args), callable_ = callable](auto &&rng) {
                    return std::invoke(callable_, FWD(rng), args_...);
                }};
        }
    }
};

template <typename Callable>
range_adaptor(Callable) -> range_adaptor<Callable>;

template <typename Callable>
struct range_adaptor_closure : range_adaptor<Callable> {
    inline constexpr auto operator()(std::ranges::viewable_range auto &&r) const requires std::invocable<Callable, decltype(r)> {
        return std::invoke(this->callable, FWD(r));
    }

    inline friend constexpr auto operator|(std::ranges::viewable_range auto &&r, range_adaptor_closure const &c) requires std::invocable<decltype(c), decltype(r)> {
        return std::invoke(c, FWD(r));
    }

    template <typename T>
    inline friend constexpr auto operator|(range_adaptor_closure<T> const &lhs, range_adaptor_closure const &rhs) {
        return range_adaptor_closure{[lhs, rhs](std::ranges::viewable_range auto &&r) {
            return FWD(r) | lhs | rhs;
        }};
    }
};

template <typename Callable>
range_adaptor_closure(Callable) -> range_adaptor_closure<Callable>;

}  // namespace cpputils::detail::adaptors

#undef FWD
#endif