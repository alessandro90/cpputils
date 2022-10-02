#ifndef CPPUTILS_FUNCTIONAL_ENUMERATE_HPP
#define CPPUTILS_FUNCTIONAL_ENUMERATE_HPP

#include "../internal/adaptors.hpp"
#include "../internal/iter_utils.hpp"
#include "../traits/cpputils_concepts.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>
// TODO: iter_move? iter_swap?
// TODO: proxy reference for post increment?
// template <class T>
// struct post_increment_proxy {
// private:
//     T cache_;

// public:
//     template <typename U>
//     constexpr post_increment_proxy(U &&t)
//         : cache_(std::forward<U>(t)) {}
//     constexpr T const &operator*() const noexcept {
//         return cache_;
//     }
// };
namespace cpputils {

template <std::ranges::input_range Range, minimal_incrementable Index = std::size_t>
requires std::copyable<Index> && std::is_default_constructible_v<Index> && std::ranges::view<Range>
class enumerate_view : public std::ranges::view_interface<enumerate_view<Range, Index>> {
public:
    struct enumeration {
        Index index;
        std::ranges::range_reference_t<Range> value;
    };

    constexpr enumerate_view() = default;

    constexpr enumerate_view(Range rng, Index start)
        : m_range{rng}
        , m_start{std::move(start)} {}

    constexpr explicit enumerate_view(Range rng)
        : enumerate_view{rng, std::size_t{0U}} {}

    class iterator {
        using iter_type = std::ranges::iterator_t<Range>;

    public:
        struct sentinel {
            [[nodiscard]] constexpr bool operator==(sentinel const &) const { return true; }

            [[nodiscard]] constexpr bool operator==(iterator const &it) const {
                return it.m_iter == it.m_last;
            }
        };
        // using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = enumeration;
        // using pointer = void;
        using reference = value_type;

        constexpr iterator() = default;

        constexpr explicit iterator(Range rng, Index index)
            : m_iter{std::ranges::begin(rng)}
            , m_last{std::ranges::end(rng)}
            , m_index{index} {}

        constexpr auto operator*() const {
            return value_type{m_index, *m_iter};
        }

        constexpr auto operator->() = delete;

        constexpr auto &operator++() {
            std::ranges::advance(m_iter, 1);
            ++m_index;
            return *this;
        }

        constexpr void operator++(int) {
            ++*this;
        }

        [[nodiscard]] constexpr bool operator==(iterator const &it) const {
            return m_iter == it.m_iter;
        }

    private:
        iter_type m_iter{};
        iter_type m_last{};
        Index m_index{};
    };

    constexpr auto begin() { return iterator{m_range, m_start}; }
    constexpr auto end() { return typename iterator::sentinel{}; }

private:
    Range m_range;
    Index m_start;
};

template <typename Range, typename Index>
enumerate_view(Range &&, Index &&) -> enumerate_view<std::ranges::views::all_t<Range>, std::remove_cvref_t<Index>>;

// clang-format off
inline constexpr auto enumerate = detail::adaptors::range_adaptor{
    []<typename T = std::size_t>(std::ranges::input_range auto &&r, T &&i = std::size_t{}) {
        return enumerate_view{std::forward<decltype(r)>(r), std::forward<decltype(i)>(i)};
    }
};
// clang-format on

}  // namespace cpputils

template <typename Range, typename Index>
inline constexpr bool std::ranges::enable_borrowed_range<::cpputils::enumerate_view<Range, Index>> = std::ranges::enable_borrowed_range<Range>;

#endif
