#include <algorithm>
#include <cstddef>
#include <string_view>

namespace cpputils {
template <std::size_t N>
struct static_string {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays, hicpp-explicit-conversions)
    constexpr explicit(false) static_string(char const (&str)[N + 1]) {
        std::ranges::copy_n(str, N, m_str);
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    [[nodiscard]] constexpr explicit(false) operator std::string_view() const {
        return {m_str, N};
    }

    inline static constexpr auto size = N;

    char m_str[N]{};  // NOLINT(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
};

template <std::size_t N>
static_string(char const (&)[N]) -> static_string<N - 1>;  // NOLINT(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
}  // namespace cpputils
