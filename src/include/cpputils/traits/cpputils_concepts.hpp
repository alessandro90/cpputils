#ifndef CPPUTILS_CONCEPTS_HPP
#define CPPUTILS_CONCEPTS_HPP

#include "is_specialization_of.hpp"
#include <concepts>
#include <type_traits>


namespace cpputils {
template <typename T, template <typename...> typename Target>
concept an = is_specialization_v<std::remove_cvref_t<T>, Target>;

namespace detail {
    template <typename T, typename Q>
    struct is_same_underlying_type
        : std::conditional_t<std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Q>>,
                             std::true_type,
                             std::false_type> {};

    template <typename T, typename Q>
    concept same_underlying_type = is_same_underlying_type<T, Q>::value;
}  // namespace detail

template <typename T>
concept minimal_incrementable = requires(T t) {
    { ++t } -> detail::same_underlying_type<T>;
};

namespace tl {
    template <template <typename...> typename Target, typename... Args>
    concept specializable = requires() {
        typename Target<Args...>;
    };

    template <typename T>
    concept predicate = requires(T) {
        { T::value } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept transformation = requires() {
        typename T::type;
    };
}  // namespace tl
}  // namespace cpputils


#endif
