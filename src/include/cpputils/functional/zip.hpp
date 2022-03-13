#ifndef CPPUTILS_FUNCTIONAL_ZIP_HPP
#define CPPUTILS_FUNCTIONAL_ZIP_HPP

#include <ranges>
#include <tuple>
#include <utility>

#include "../internal/iter_utils.hpp"


namespace cpputils {
// clang-format off
template <std::ranges::view... Containers>
requires(std::ranges::input_range<Containers> &&...) &&
        (sizeof...(Containers) > 0)
class zip : public std::ranges::view_interface<zip<Containers...>> {
    // clang-format on
    using tuple_type = std::tuple<Containers...>;

public:
    constexpr zip() = default;
    explicit constexpr zip(Containers... containers)
        : m_data{containers...} {}


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
        using value_type = detail::tuple_ref<Containers...>;
        using reference = value_type;
        // using pointer = void;

        constexpr iterator() = default;

        explicit constexpr iterator(tuple_type const &ref)
            : m_tup{ref}
            , m_it_tup{detail::make_iter_tuple(ref, detail::iseq<Containers...>())} {}

        constexpr auto operator*() const { return deref(detail::iseq<Containers...>()); }

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
        tuple_type m_tup{};
        tuple_it_type m_it_tup{};

        template <std::size_t... I>
        constexpr auto deref(std::index_sequence<I...>) const {
            return value_type{*std::get<I>(m_it_tup)...};
        }
    };

    constexpr auto begin() { return iterator{m_data}; }
    constexpr auto end() { return typename iterator::sentinel{}; }

private:
    tuple_type m_data;
};

template <typename... Containers>
zip(Containers &&...) -> zip<std::ranges::views::all_t<Containers>...>;
}  // namespace cpputils

template <typename... Containers>
inline constexpr bool std::ranges::enable_borrowed_range<::cpputils::zip<Containers...>> = (std::ranges::enable_borrowed_range<Containers> && ...);

#endif
