#ifndef CPPUTILS_TOVECTOR_HPP
#define CPPUTILS_TOVECTOR_HPP

#include "../internal/adaptors.hpp"
#include <algorithm>
#include <concepts>
#include <ranges>
#include <utility>
#include <vector>


// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {

inline constexpr auto to_vector = detail::adaptors::range_adaptor{
    [](std::ranges::input_range auto &&r) /* clang-format off */ requires std::indirectly_copyable<std::ranges::iterator_t<decltype(r)>, std::ranges::iterator_t<std::vector<std::ranges::range_value_t<decltype(r)>>>> /* clang-format off */ {
        std::vector<std::ranges::range_value_t<decltype(r)>> v;
        if constexpr (requires { std::ranges::size(r); }) {
            v.reserve(std::ranges::size(r));
        }
        
        std::ranges::copy(FWD(r), std::back_inserter(v));
        
        return v;
    }};

}  // namespace cpputils

#undef FWD

#endif
