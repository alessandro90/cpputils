#ifndef CPPUTILS_TYPELIST_HPP
#define CPPUTILS_TYPELIST_HPP

#include "cpputils/traits/cpputils_concepts.hpp"
#include "cpputils/traits/is_specialization_of.hpp"
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace cpputils::tl {

template <typename... Ts>
struct typelist {};

template <typename...>
struct is_typelist : std::false_type {};

template <typename T>
inline constexpr auto is_typelist_v = is_specialization_v<T, typelist>;

struct null {};

template <typename>
struct inner;

template <>
struct inner<typelist<>> {
    using type = null;
};

template <typename T>
struct inner<typelist<T>> {
    using type = T;
};

// Head function
template <typename...>
struct head;

template <>
struct head<typelist<>> {
    using list = typelist<>;
};

template <typename T, typename... Ts>
struct head<typelist<T, Ts...>> {
    using list = typelist<T>;
};

template <>
struct head<> {
    template <typename... Ts>
    using list = head<Ts...>::list;
};

// Tail function
template <typename...>
struct tail;

template <>
struct tail<typelist<>> {
    using list = typelist<>;
};

template <typename T, typename... Ts>
struct tail<typelist<T, Ts...>> {
    using list = typelist<Ts...>;
};

template <>
struct tail<> {
    template <an<typelist> T>
    using list = tail<T>::list;
};

// join
template <typename...>
struct join;

template <typename... Ts, typename... Gs>
struct join<typelist<Ts...>, typelist<Gs...>> {
    using list = typelist<Ts..., Gs...>;
};

template <typename... Ts, typename... Es, an<typelist>... rest>
struct join<typelist<Ts...>, typelist<Es...>, rest...> {
    using list = join<typelist<Ts..., Es...>, rest...>::list;
};


// Prepend
template <typename...>
struct prepend;

template <typename T, typename... Ts>
struct prepend<T, typelist<Ts...>> {
    using list = typelist<T, Ts...>;
};

template <typename T>
struct prepend<T> {
    template <an<typelist> Typelist>
    using list = prepend<T, Typelist>::list;
};

// append
template <typename...>
struct append;

template <typename T, typename... Ts>
struct append<T, typelist<Ts...>> {
    using list = typelist<Ts..., T>;
};


template <typename T>
struct append<T> {
    template <an<typelist> Typelist>
    using list = append<T, Typelist>::list;
};

// Take function
template <std::size_t, typename...>
struct take;

template <typename... Ts>
struct take<0U, typelist<Ts...>> {
    using list = typelist<>;
};

template <std::size_t N, typename... Ts>
struct take<N, typelist<Ts...>> {
    using list = join<typename head<typelist<Ts...>>::list,
                      typename take<N - 1, typename tail<typelist<Ts...>>::list>::list>::list;
};

template <std::size_t N>
struct take<N> {
    template <an<typelist> T>
    using list = take<N, T>::list;
};

// as
template <template <typename...> typename, typename...>
struct as;

template <template <typename...> typename Target, typename... Ts>
requires specializable<Target, Ts...>
struct as<Target, typelist<Ts...>> {
    using type = Target<Ts...>;
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
    using list = typelist<>;
};

template <typename T, typename... Ts>
struct last<typelist<T, Ts...>> {
    using list = std::conditional_t<sizeof...(Ts) == 0,
                                    typelist<T>,
                                    typename last<typelist<Ts...>>::list>;
};

template <>
struct last<> {
    template <an<typelist> T>
    using list = last<T>::list;
};

// pop_front
template <typename, typename...>
struct pop_front;

template <>
struct pop_front<typelist<>> {
    using list = typelist<>;
    using front = typelist<>;
};

template <typename... Ts>
struct pop_front<typelist<Ts...>> {
    using list = take<len<typelist<Ts...>>::value - 1, typelist<Ts...>>::list;
    using front = last<typelist<Ts...>>::list;
};

// pop_back

template <typename, typename...>
struct pop_back;

template <>
struct pop_back<typelist<>> {
    using list = typelist<>;
    using back = typelist<>;
};

template <typename... Ts>
struct pop_back<typelist<Ts...>> {
    using list = tail<typelist<Ts...>>::list;
    using back = head<typelist<Ts...>>::list;
};

// reverse
template <typename...>
struct reverse;

template <>
struct reverse<typelist<>> {
    using list = typelist<>;
};

template <typename... Ts>
struct reverse<typelist<Ts...>> {
    using list = join<
        typename last<typelist<Ts...>>::list,
        typename reverse<
            typename pop_front<
                typelist<Ts...>>::list>::list>::list;
};

template <>
struct reverse<> {
    template <an<typelist> T>
    using list = reverse<T>::list;
};

// filter
template <template <typename> typename F, typename... Ts>
struct filter;

template <template <typename> typename F>
struct filter<F, typelist<>> {
    using list = typelist<>;
};

template <template <typename> typename F, typename... Ts>
requires(predicate<F<Ts>> &&...) struct filter<F, typelist<Ts...>> {
    using list = std::conditional_t<
        F<typename inner<typename head<typelist<Ts...>>::list>::type>::value,
        typename join<
            typename head<typelist<Ts...>>::list,
            typename filter<F, typename tail<typelist<Ts...>>::list>::list>::list,
        typename filter<F, typename tail<typelist<Ts...>>::list>::list>;
};

template <template <typename> typename F>
struct filter<F> {
    template <an<typelist> T>
    using list = filter<F, T>::list;
};

// repeat<T, N>
namespace detail {
    template <std::size_t N, typename T, typename... Ts>
    struct repeat_impl;

    template <typename T, typename... Ts>
    struct repeat_impl<0U, T, typelist<Ts...>> {
        using list = typelist<Ts...>;
    };

    template <std::size_t N, typename T, typename... Ts>
    struct repeat_impl<N, T, typelist<Ts...>> {
        using list = detail::repeat_impl<N - 1, T, typename append<T, typelist<Ts...>>::list>::list;
    };
}  // namespace detail

template <std::size_t N, typename T>
using repeat = detail::repeat_impl<N, T, typelist<>>;

// take while
template <template <typename> typename F, typename... Ts>
struct take_while;

template <template <typename> typename F>
struct take_while<F, typelist<>> {
    using list = typelist<>;
};

template <template <typename> typename F, typename... Ts>
requires(predicate<F<Ts>> &&...) struct take_while<F, typelist<Ts...>> {
    using list = std::conditional_t<
        F<typename inner<typename head<typelist<Ts...>>::list>::type>::value,
        typename join<typename head<typelist<Ts...>>::list,
                      typename take_while<F, typename tail<typelist<Ts...>>::list>::list>::list,
        typelist<>>;
};

template <template <typename> typename F>
struct take_while<F> {
    template <an<typelist> T>
    using list = take_while<F, T>::list;
};

// flat
template <typename...>
struct flat;

template <>
struct flat<typelist<>> {
    using list = typelist<>;
};

template <typename T, typename... Ts>
struct flat<typelist<T, Ts...>> {
    using list = join<typelist<T>, typename flat<typelist<Ts...>>::list>::list;
};

template <an<typelist> T, typename... Ts>
struct flat<typelist<T, Ts...>> {
    using list = join<typename flat<T>::list, typename flat<typelist<Ts...>>::list>::list;
};

template <>
struct flat<> {
    template <an<typelist> T>
    using list = flat<T>::list;
};

// zip
template <typename...>
struct zip;

template <an<typelist>... lists>
requires((empty<lists>::value && ...) && sizeof...(lists) > 1) struct zip<lists...> {
    using list = typelist<>;
};

namespace detail {
    template <an<typelist> T, an<typelist>... Ts>
    struct same_len
        : std::conditional_t<((len<T>::value == len<Ts>::value) && ...),
                             std::true_type,
                             std::false_type> {};
}  // namespace detail

template <an<typelist>... lists>
requires((!empty<lists>::value && ...)
         && detail::same_len<lists...>::value
         && sizeof...(lists) > 1) struct zip<lists...> {
    using list = prepend<typename join<typename head<lists>::list...>::list,
                         typename zip<typename tail<lists>::list...>::list>::list;
};

// unique
namespace detail {
    template <typename...>
    struct unique_impl;
    template <typename... Ts>
    struct unique_impl<typelist<Ts...>, typelist<>> {
        using list = typelist<Ts...>;
    };
    template <typename... Es, typename T, typename... Ts>
    struct unique_impl<typelist<Es...>, typelist<T, Ts...>> {
        using list = std::conditional_t<std::disjunction_v<std::is_same<T, Es>...>,
                                        typename unique_impl<typelist<Es...>, typelist<Ts...>>::list,
                                        typename unique_impl<typelist<Es..., T>, typelist<Ts...>>::list>;
    };

}  // namespace detail

template <typename...>
struct unique;

template <an<typelist> T>
struct unique<T> {
    using list = detail::unique_impl<typelist<>, T>::list;
};

template <>
struct unique<> {
    template <an<typelist> T>
    using list = unique<T>::list;
};

// map

namespace detail {
    template <template <typename> typename F, typename... Ts>
    struct map_impl;

    template <template <typename> typename F, typename... Es>
    struct map_impl<F, typelist<Es...>, typelist<>> {
        using list = typelist<Es...>;
    };

    template <template <typename> typename F, typename... Es, typename T, typename... Ts>
    struct map_impl<F, typelist<Es...>, typelist<T, Ts...>> {
        using list = map_impl<F, typelist<Es..., typename F<T>::type>, typelist<Ts...>>::list;
    };
}  // namespace detail

template <template <typename> typename F, typename... Ts>
struct map;

template <template <typename> typename F, typename... Ts>
requires(transformation<F<Ts>> &&...) struct map<F, typelist<Ts...>> {
    using list = detail::map_impl<F, typelist<>, typelist<Ts...>>::list;
};

template <template <typename> typename F>
struct map<F> {
    template <an<typelist> T>
    using list = map<F, T>::list;
};

// count
template <typename, typename...>
struct count;

template <typename T>
struct count<T, typelist<>> {
    inline static constexpr std::size_t value = 0;
};

template <typename T, typename... Ts>
struct count<T, typelist<Ts...>> {
    inline static constexpr std::size_t value =
        (std::is_same_v<T, typename inner<typename head<typelist<Ts...>>::list>::type> ? 1 : 0)
        + count<T, typename tail<typelist<Ts...>>::list>::value;
};

// count if
template <template <typename> typename, typename...>
struct count_if;

template <template <typename> typename P>
struct count_if<P, typelist<>> {
    inline static constexpr std::size_t value = 0;
};

template <template <typename> typename P, typename... Ts>
requires(predicate<P<Ts>> &&...) struct count_if<P, typelist<Ts...>> {
    inline static constexpr std::size_t value =
        (P<typename inner<typename head<typelist<Ts...>>::list>::type>::value ? 1 : 0)
        + count_if<P, typename tail<typelist<Ts...>>::list>::value;
};

// drop
template <std::size_t, typename...>
struct drop;

template <typename... Ts>
struct drop<0U, typelist<Ts...>> {
    using list = typelist<Ts...>;
};

template <std::size_t N, typename... Ts>
struct drop<N, typelist<Ts...>> {
    using list = drop<N - 1, typename tail<typelist<Ts...>>::list>::list;
};

template <std::size_t N>
struct drop<N> {
    template <an<typelist> T>
    using list = drop<N, T>::list;
};

// drop while
template <template <typename> typename P, typename...>
struct drop_while;

template <template <typename> typename P>
struct drop_while<P, typelist<>> {
    using list = typelist<>;
};

template <template <typename> typename P, typename... Ts>
requires(predicate<P<Ts>> &&...) struct drop_while<P, typelist<Ts...>> {
    using list = std::conditional_t<P<typename inner<typename head<typelist<Ts...>>::list>::type>::value,
                                    typename drop_while<P, typename tail<typelist<Ts...>>::list>::list,
                                    typelist<Ts...>>;
};

template <template <typename> typename P>
struct drop_while<P> {
    template <an<typelist> T>
    using list = drop_while<P, T>::list;
};

// enumerate
template <std::size_t I, typename T>
struct enumeration {
    using type = T;
    inline static constexpr auto index = I;
};

template <typename...>
struct enumerate;

namespace detail {
    template <typename, typename>
    struct enumerate_impl;

    template <std::size_t... Is, typename... Ts>
    struct enumerate_impl<std::index_sequence<Is...>, typelist<Ts...>> {
        using list = typelist<enumeration<Is, Ts>...>;
    };
}  // namespace detail

template <>
struct enumerate<typelist<>> {
    using list = typelist<>;
};

template <typename... Ts>
struct enumerate<typelist<Ts...>> {
    using list = detail::enumerate_impl<std::index_sequence_for<Ts...>,
                                        typelist<Ts...>>::list;
};

template <>
struct enumerate<> {
    template <an<typelist> T>
    using list = enumerate<T>::list;
};

// zipwith
template <template <typename...> typename, typename...>
struct zip_with;

template <template <typename...> typename F, an<typelist>... lists>
requires((empty<lists>::value && ...) && sizeof...(lists) > 1) struct zip_with<F, lists...> {
    using list = typelist<>;
};

template <template <typename...> typename F, an<typelist>... lists>
requires((!empty<lists>::value && ...)
         && detail::same_len<lists...>::value
         && sizeof...(lists) > 1) struct zip_with<F, lists...> {
    using list = join<typelist<typename F<typename inner<typename head<lists>::list>::type...>::type>,
                      typename zip_with<F, typename tail<lists>::list...>::list>::list;
};

// compose
namespace detail {
    template <typename Closure, typename Typelist>
    concept closure = an<Typelist, typelist> && requires() {
        typename Closure::list<Typelist>;
    };

    template <an<typelist> T, closure<T> Closure, typename... Closures>
    struct compose_impl {
        using list = compose_impl<typename Closure::list<T>, Closures...>::list;
    };

    template <an<typelist> T, closure<T> Closure>
    struct compose_impl<T, Closure> {
        using list = typename Closure::list<T>;
    };
}  // namespace detail

template <typename... Closures>
struct compose {
    template <an<typelist> T>
    using list = detail::compose_impl<T, Closures...>::list;
};

// fold
namespace detail {
    template <template <typename, typename> typename, typename...>
    struct fold_impl;

    template <template <typename, typename> typename F, typename T>
    struct fold_impl<F, T, typelist<>> {
        using list = typelist<T>;
    };

    template <template <typename, typename> typename F, typename T, typename... Ts>
    struct fold_impl<F, T, typelist<Ts...>> {
        using list = fold_impl<F,
                               typename F<T, typename inner<typename head<typelist<Ts...>>::list>::type>::type,
                               typename tail<typelist<Ts...>>::list>::list;
    };
}  // namespace detail

template <template <typename, typename> typename F, typename... Ts>
struct fold;

template <template <typename, typename> typename F>
struct fold<F, typelist<>> {
    using list = typelist<>;
};

template <template <typename, typename> typename F, typename... Ts>
struct fold<F, typelist<Ts...>> {
    using list = detail::fold_impl<F,
                                   typename head<typelist<Ts...>>::list,
                                   typename tail<typelist<Ts...>>::list>::list;
};

template <template <typename, typename> typename F>
struct fold<F> {
    template <an<typelist> T>
    using list = fold<F, T>::list;
};

// compose_predicate
template <template <typename> typename... P>
struct compose_predicate {
    template <typename T>
    requires(predicate<P<T>> &&...) struct composed
        : std::conjunction<P<T>...> {};
};

// compose_trasformation
template <template <typename> typename E, template <typename> typename... F>
struct compose_transformation {
    template <typename T>
    requires transformation<E<T>>
    struct composed {
        using type = typename compose_transformation<F...>::composed<typename E<T>::type>::type;
    };
};

template <template <typename> typename F>
struct compose_transformation<F> {
    template <typename T>
    requires transformation<F<T>>
    struct composed {
        using type = typename F<T>::type;
    };
};

// keep (integers)
// get (integer)
}  // namespace cpputils::tl
#endif
