#ifndef CPPUTILS_CONCEPTS_HPP
#define CPPUTILS_CONCEPTS_HPP

#include "is_specialization_of.hpp"
#include <concepts>
#include <type_traits>


namespace cpputils {
template <typename T, template <typename...> typename Target>
concept an = is_specialization_v<std::remove_cvref_t<T>, Target>;

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
