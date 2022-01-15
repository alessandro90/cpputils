#ifndef CPPUTILS_TYPELIST_HPP
#define CPPUTILS_TYPELIST_HPP

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace cpputils::tl {

template <typename... Ts>
struct typelist {
};

template <typename>
struct inner;

template <typename T>
struct inner<typelist<T>> {
    using type = T;
};

// Head function
template <typename...>
struct head;

template <>
struct head<typelist<>> {
    using boxed = typelist<>;
};

template <typename T, typename... Ts>
struct head<typelist<T, Ts...>> {
    using boxed = typelist<T>;
};

// Tail function
template <typename...>
struct tail;

template <>
struct tail<typelist<>> {
    using boxed = typelist<>;
};

template <typename T, typename... Ts>
struct tail<typelist<T, Ts...>> {
    using boxed = typelist<Ts...>;
};

// Merge
template <typename...>
struct merge;

template <typename... Ts, typename... Gs>
struct merge<typelist<Ts...>, typelist<Gs...>> {
    using boxed = typelist<Ts..., Gs...>;
};

// Prepend
template <typename...>
struct prepend;

template <typename T, typename... Ts>
struct prepend<T, typelist<Ts...>> {
    using boxed = typelist<T, Ts...>;
};

// append
template <typename...>
struct append;

template <typename T, typename... Ts>
struct append<T, typelist<Ts...>> {
    using boxed = typelist<Ts..., T>;
};

// Take function
template <std::size_t, typename...>
struct take;

template <typename... Ts>
struct take<0U, typelist<Ts...>> {
    using boxed = typelist<>;
};

template <std::size_t N, typename... Ts>
struct take<N, typelist<Ts...>> {
    using boxed = typename merge<typename head<typelist<Ts...>>::boxed,
                                 typename take<N - 1, typename tail<typelist<Ts...>>::boxed>::boxed>::boxed;
};

// as tuple
template <typename...>
struct as_tuple;

template <typename... Ts>
struct as_tuple<typelist<Ts...>> {
    using boxed = std::tuple<Ts...>;
};

// len
template <typename...>
struct len;

template <typename... Ts>
struct len<typelist<Ts...>> {
    inline static constexpr auto value = sizeof...(Ts);
};

// Empty
template <typename...>
struct empty;

template <>
struct empty<typelist<>> : std::true_type {
};

template <typename... Ts>
struct empty<typelist<Ts...>> : std::false_type {
};

// last
template <typename...>
struct last;

template <>
struct last<typelist<>> {
    using boxed = typelist<>;
};

template <typename T, typename... Ts>
struct last<typelist<T, Ts...>> {
    using boxed = typename std::conditional_t<sizeof...(Ts) == 0, typelist<T>, typename last<typelist<Ts...>>::boxed>;
};

// pop_front
template <typename, typename...>
struct pop_front;

template <>
struct pop_front<typelist<>> {
    using boxed = typelist<>;
    using popped = typelist<>;
};

template <typename... Ts>
struct pop_front<typelist<Ts...>> {
    using boxed = typename take<len<typelist<Ts...>>::value - 1, typelist<Ts...>>::boxed;
    using popped = typename last<typelist<Ts...>>::boxed;
};

// pop_back

template <typename, typename...>
struct pop_back;

template <>
struct pop_back<typelist<>> {
    using boxed = typelist<>;
    using popped = typelist<>;
};

template <typename... Ts>
struct pop_back<typelist<Ts...>> {
    using boxed = typename tail<typelist<Ts...>>::boxed;
    using popped = typename head<typelist<Ts...>>::boxed;
};

// reverse
template <typename...>
struct reverse;

template <>
struct reverse<typelist<>> {
    using boxed = typelist<>;
};

template <typename... Ts>
struct reverse<typelist<Ts...>> {
    using boxed = typename merge<
        typename last<typelist<Ts...>>::boxed,
        typename reverse<
            typename pop_front<
                typelist<Ts...>>::boxed>::boxed>::boxed;
};

// filter

template <typename T>
concept predicate = requires(T) {
    {
        T::value
        }
        -> std::convertible_to<bool>;
};

template <template <typename> typename F, typename... Ts>
struct filter;

template <template <typename> typename F>
struct filter<F, typelist<>> {
    using boxed = typelist<>;
};

template <template <typename> typename F, typename... Ts>
requires(predicate<F<Ts>> &&...) struct filter<F, typelist<Ts...>> {
    using boxed = std::conditional_t<
        F<typename inner<typename head<typelist<Ts...>>::boxed>::type>::value,
        typename merge<
            typename head<typelist<Ts...>>::boxed,
            typename filter<F, typename tail<typelist<Ts...>>::boxed>::boxed>::boxed,
        typename filter<F, typename tail<typelist<Ts...>>::boxed>::boxed>;
};

// repeat<T, N>
template <std::size_t N, typename T, typename... Ts>
struct repeat_impl;

template <typename T, typename... Ts>
struct repeat_impl<0U, T, typelist<Ts...>> {
    using boxed = typelist<Ts...>;
};

template <std::size_t N, typename T, typename... Ts>
struct repeat_impl<N, T, typelist<Ts...>> {
    using boxed = typename repeat_impl<N - 1, T, typename append<T, typelist<Ts...>>::boxed>::boxed;
};

template <std::size_t N, typename T>
using repeat = repeat_impl<N, T, typelist<>>;

// take while
// drop
// drop while
// map
// count
// count if
// zip
// zipwith
// unique
}  // namespace cpputils::tl
#endif