#ifndef CPPUTILS_PIPEABLE_HPP
#define CPPUTILS_PIPEABLE_HPP

#include "../internal/adaptors.hpp"
#include <ranges>
#include <utility>

// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {
constexpr auto pipeable(auto adaptor) {
    return detail::adaptors::range_adaptor_closure{[adaptor_ = std::move(adaptor)](std::ranges::viewable_range auto &&r) {
        return adaptor_(FWD(r));
    }};
}
}  // namespace cpputils

#undef FWD

#endif
