#ifndef CPPUTILS_CONTAINER_VIEWS_HPP
#define CPPUTILS_CONTAINER_VIEWS_HPP

#include <array>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>


namespace cpputils {

struct with_fixed_extent_t {};

inline constexpr auto with_fixed_extent = with_fixed_extent_t{};

template <typename C>
concept character =
    std::same_as<C, char>
    || std::same_as<C, signed char>
    || std::same_as<C, unsigned char>
    || std::same_as<C, wchar_t>
    || std::same_as<C, char16_t>
    || std::same_as<C, char32_t>
    || std::same_as<C, char8_t>;

// Dummy conversion for character literals. Implicit conversion to string view and just return it (did not find a better way yet, cannot use templates)
// [[nodiscard]] constexpr auto as_view(std::string const &s) -> std::basic_string_view<char> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<char> const &s) -> std::basic_string_view<char> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<signed char> const &s) -> std::basic_string_view<signed char> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<unsigned char> const &s) -> std::basic_string_view<unsigned char> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<wchar_t> const &s) -> std::basic_string_view<wchar_t> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<char16_t> const &s) -> std::basic_string_view<char16_t> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<char32_t> const &s) -> std::basic_string_view<char32_t> { return s; }
[[nodiscard]] constexpr auto as_view(std::basic_string<char8_t> const &s) -> std::basic_string_view<char8_t> { return s; }

[[nodiscard]] constexpr auto as_view(char const *s) -> std::basic_string_view<char> { return s; }
[[nodiscard]] constexpr auto as_view(signed char const *s) -> std::basic_string_view<signed char> { return s; }
[[nodiscard]] constexpr auto as_view(unsigned char const *s) -> std::basic_string_view<unsigned char> { return s; }
[[nodiscard]] constexpr auto as_view(wchar_t const *s) -> std::basic_string_view<wchar_t> { return s; }
[[nodiscard]] constexpr auto as_view(char16_t const *s) -> std::basic_string_view<char16_t> { return s; }
[[nodiscard]] constexpr auto as_view(char32_t const *s) -> std::basic_string_view<char32_t> { return s; }
[[nodiscard]] constexpr auto as_view(char8_t const *s) -> std::basic_string_view<char8_t> { return s; }

namespace detail {
    template <std::size_t N>
    constexpr auto make_span_with_fixed_extent(std::ranges::range auto &&range) {
        using range_value = std::ranges::range_value_t<decltype(range)>;
        using value_type = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(range)>>,
                                              std::add_const_t<range_value>,
                                              range_value>;
        return std::span<value_type, N>{range};
    }
}  // namespace detail

template <with_fixed_extent_t, typename T, std::size_t N>
[[nodiscard]] constexpr auto as_view(std::array<T, N> const &a) -> std::span<T const, N> {
    return detail::make_span_with_fixed_extent<N>(a);
}

template <with_fixed_extent_t, typename T, std::size_t N>
[[nodiscard]] constexpr auto as_view(std::array<T, N> &a) -> std::span<T, N> {
    return detail::make_span_with_fixed_extent<N>(a);
}

template <with_fixed_extent_t, typename T, std::size_t N>
constexpr auto as_view(std::array<T, N> &&a) = delete;

template <with_fixed_extent_t, typename T, std::size_t N>
[[nodiscard]] constexpr auto as_view(T const (&a)[N]) -> std::span<T const, N> {  // NOLINT
    return detail::make_span_with_fixed_extent<N>(a);
}

template <with_fixed_extent_t, typename T, std::size_t N>
[[nodiscard]] constexpr auto as_view(T (&a)[N]) -> std::span<T const, N> {  // NOLINT
    return detail::make_span_with_fixed_extent<N>(a);
}

template <with_fixed_extent_t, typename T, std::size_t N>
constexpr auto as_view(T (&&a)[N]) = delete;  // NOLINT

[[nodiscard]] constexpr auto as_view(std::ranges::contiguous_range auto const &range) -> std::span<std::add_const_t<std::ranges::range_value_t<decltype(range)>>> requires std::ranges::sized_range<decltype(range)>
{
    return std::span{range};
}

[[nodiscard]] constexpr auto as_view(std::ranges::contiguous_range auto &range) -> std::span<std::ranges::range_value_t<decltype(range)>> requires std::ranges::sized_range<decltype(range)>
{
    return std::span{range};
}

constexpr auto as_view(std::ranges::contiguous_range auto &&range) requires std::ranges::sized_range<decltype(range)>
= delete;

}  // namespace cpputils

#endif
