#ifndef CPPUTILS_TRAITS_HPP
#define CPPUTILS_TRAITS_HPP

#include <chrono>
#include <concepts>
#include <type_traits>


namespace cpputils {
namespace detail {
    template <typename, template <typename...> typename>
    struct is_specialization : public std::false_type {};

    template <typename... Ts, template <typename...> typename U>
    struct is_specialization<U<Ts...>, U> : public std::true_type {};
}  // namespace detail

template <typename T, template <typename...> typename U>
struct is_specialization
    : public detail::is_specialization<std::remove_cvref_t<T>, U> {};

template <typename T, template <typename...> typename U>
inline constexpr auto is_specialization_v = is_specialization<T, U>::value;

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
concept minimal_incrementable =
    requires (T t) {
        { ++t } -> detail::same_underlying_type<T>;
    };

namespace tl {
    template <template <typename...> typename Target, typename... Args>
    concept specializable =
        requires () {
            typename Target<Args...>;
        };

    template <typename T>
    concept predicate =
        requires (T) {
            { T::value } -> std::convertible_to<bool>;
        };

    template <typename T>
    concept transformation =
        requires () {
            typename T::type;
        };
}  // namespace tl

template <typename>
struct is_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <typename D>
inline constexpr auto is_duration_v = is_duration<D>::value;

template <typename D>
concept duration = is_duration_v<D>;

template <typename T>
concept pair_like =
    requires (T t) {
        t.first;
        t.second;
    }
    && std::is_member_object_pointer_v<std::remove_cvref_t<decltype(&std::remove_cvref_t<T>::first)>>
    && std::is_member_object_pointer_v<std::remove_cvref_t<decltype(&std::remove_cvref_t<T>::second)>>;
}  // namespace cpputils

#endif
