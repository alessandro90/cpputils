#ifndef CPPUTILS_LAZY_HPP
#define CPPUTILS_LAZY_HPP

#include <concepts>
#include <tuple>
#include <utility>

namespace cpputils {

template <typename T, typename... Args>
requires(std::constructible_from<T, std::remove_cvref_t<Args>...> &&  //
         !std::disjunction_v<std::is_reference<Args>...>) class [[nodiscard]] Lazy {
public:
    explicit constexpr Lazy(Args... args) requires(sizeof...(Args) > 0)
        : m_args{std::move(args)...} {
    }

    Lazy() = default;

    [[nodiscard]] constexpr T build() const {
        return std::make_from_tuple<T>(std::move(m_args));
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    [[nodiscard]] constexpr operator T() const {
        return build();
    }

private:
    std::tuple<Args...> m_args{};
};

template <typename T, typename... Args>
constexpr auto lazy(Args... args) {
    return Lazy<T, Args...>{std::move(args)...};
}
}  // namespace cpputils

#endif
