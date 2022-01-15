#ifndef CPPUTILS_FUNCTIONAL_ITER_UTILS_HPP
#define CPPUTILS_FUNCTIONAL_ITER_UTILS_HPP

#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>


namespace cpputils::detail {

template <std::ranges::input_range... Iterables>
using tuple_ref = std::tuple<std::ranges::range_reference_t<Iterables>...>;

template <std::ranges::input_range... Iterables>
using tuple_iter = std::tuple<std::ranges::iterator_t<Iterables>...>;

template <typename... Objects>
constexpr auto iseq() { return std::make_index_sequence<sizeof...(Objects)>{}; }

// template <std::size_t I, std::ranges::input_range... Iterables, std::input_iterator... Iterators>
// constexpr bool check_end_(std::tuple<Iterables...> const &tup, std::tuple<Iterators...> const &iters_tup) {
//     if constexpr (I < sizeof...(Iterables)) {
//         if (std::ranges::end(std::get<I>(tup)) == std::get<I>(iters_tup)) {
//             return true;
//         }
//         return check_end_<I + 1>(tup, iters_tup);
//     } else {
//         return false;
//     }
// }

template <std::ranges::input_range... Iterables, std::input_iterator... Iterators, std::size_t... I>
constexpr bool check_end_(std::tuple<Iterables...> const &tup, std::tuple<Iterators...> const &iters_tup, std::index_sequence<I...>) {
    return (... || (std::ranges::end(std::get<I>(tup)) == std::get<I>(iters_tup)));
}

template <std::ranges::input_range... Iterables, std::input_iterator... Iterators>
constexpr bool check_end(std::tuple<Iterables...> const &tup, std::tuple<Iterators...> const &iters_tup) {
    return check_end_(tup, iters_tup, iseq<Iterables...>());
}

template <std::ranges::input_range... Iterables, std::size_t... I>
constexpr auto make_iter_tuple(std::tuple<Iterables...> const &tup, std::index_sequence<I...>) {
    return std::tuple{std::ranges::begin(std::get<I>(tup))...};
}

template <std::input_iterator... Iterators, std::size_t... I>
constexpr auto advance_iter_tuple(std::tuple<Iterators...> const &iter_tup, std::index_sequence<I...>) {
    return std::tuple{std::ranges::next(std::get<I>(iter_tup))...};
}

}  // namespace cpputils::detail

#endif
