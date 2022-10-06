#ifndef CPPUTILS_FUNCTIONAL_UTILS_HPP
#define CPPUTILS_FUNCTIONAL_UTILS_HPP

#include <concepts>
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
}  // namespace cpputils

#undef FWD

#endif
