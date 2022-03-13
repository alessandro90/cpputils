#ifndef CPPUTILS_FUNCTIONAL_ZIP_WITH_HPP
#define CPPUTILS_FUNCTIONAL_ZIP_WITH_HPP

#include <concepts>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../internal/iter_utils.hpp"

namespace cpputils {
// clang-format off
template <typename Func, std::ranges::view... Containers>
requires (std::ranges::input_range<Containers> &&...) &&
         std::regular_invocable<Func, std::ranges::range_reference_t<Containers>...> &&
         (sizeof...(Containers) > 0)
class zip_with : public std::ranges::view_interface<zip_with<Func, Containers...>> {
    // clang-format on
    using tuple_type = std::tuple<Containers...>;

public:
    constexpr zip_with() = default;
    explicit constexpr zip_with(Func func, Containers... containers)
        : m_func{func}
        , m_data{containers...} {}


    class iterator {
        using tuple_it_type = detail::tuple_iter<Containers...>;

    public:
        struct sentinel {
            [[nodiscard]] constexpr bool operator==(sentinel const &) const { return true; }

            [[nodiscard]] constexpr bool operator==(iterator const &it) const {
                return detail::check_end(it.m_tup, it.m_it_tup);
            }
        };
        // using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::invoke_result_t<Func, std::ranges::range_reference_t<Containers>...>;
        // using pointer = void;
        // TODO: should this be defined?
        // using reference = value_type;

        constexpr iterator() = default;

        constexpr iterator(Func const &func, tuple_type const &ref)
            : m_func_iter{func}
            , m_tup{ref}
            , m_it_tup{detail::make_iter_tuple(ref, detail::iseq<Containers...>())} {}

        constexpr decltype(auto) operator*() const { return call(detail::iseq<Containers...>()); }

        constexpr auto operator->() = delete;

        constexpr auto &operator++() {
            m_it_tup = detail::advance_iter_tuple(m_it_tup, detail::iseq<Containers...>());
            return *this;
        }

        constexpr void operator++(int) {
            ++*this;
        }

        [[nodiscard]] constexpr bool operator==(iterator const &it) const {
            return m_it_tup == it.m_it_tup;
        }

    private:
        Func m_func_iter{};
        tuple_type m_tup{};
        tuple_it_type m_it_tup{};

        template <std::size_t... I>
        constexpr decltype(auto) call(std::index_sequence<I...>) const {
            return std::invoke(m_func_iter, *std::get<I>(m_it_tup)...);
        }
    };

    constexpr auto begin() { return iterator{m_func, m_data}; }
    constexpr auto end() { return typename iterator::sentinel{}; }

private:
    Func m_func;
    tuple_type m_data;
};


template <typename Func, typename... Containers>
zip_with(Func, Containers &&...) -> zip_with<Func, std::ranges::views::all_t<Containers>...>;

}  // namespace cpputils

template <typename Func, typename... Containers>
inline constexpr bool std::ranges::enable_borrowed_range<::cpputils::zip_with<Func, Containers...>> = (std::ranges::enable_borrowed_range<Containers> && ...);

#endif
