#ifndef CPPUTILS_IS_APECIALIZATION_OF_HPP
#define CPPUTILS_IS_APECIALIZATION_OF_HPP

#include <type_traits>

namespace cpputils {
template <typename...>
struct is {
    template <template <typename...> typename>
    struct specialization_of : std::false_type {};

    template <template <typename...> typename U>
    inline static constexpr auto specialization_of_v = specialization_of<U>::value;
};

template <template <typename...> typename T, typename... ts>
struct is<T<ts...>> {
    template <template <typename...> typename U>
    struct specialization_of
        : std::conditional_t<
              std::is_same_v<std::remove_cvref_t<T<ts...>>, std::remove_cvref_t<U<ts...>>>,
              std::true_type,
              std::false_type> {};

    template <template <typename...> typename U>
    inline static constexpr auto specialization_of_v = specialization_of<U>::value;
};

}  // namespace cpputils
#endif
