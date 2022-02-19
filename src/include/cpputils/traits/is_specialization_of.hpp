#ifndef CPPUTILS_IS_APECIALIZATION_OF_HPP
#define CPPUTILS_IS_APECIALIZATION_OF_HPP

#include <type_traits>

namespace cpputils {

namespace detail {
    template <typename, template <typename...> typename>
    struct is_specialization : public std::false_type {};

    template <typename... Ts, template <typename...> typename U>
    struct is_specialization<U<Ts...>, U> : public std::true_type {};
}  // namespace detail

template <typename T, template <typename...> typename U>
struct is_specialization : public detail::is_specialization<std::remove_cvref_t<T>, U> {};

template <typename T, template <typename...> typename U>
inline constexpr auto is_specialization_v = is_specialization<T, U>::value;

}  // namespace cpputils
#endif
