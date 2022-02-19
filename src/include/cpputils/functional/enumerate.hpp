#ifndef CPPUTILS_FUNCTIONAL_ENUMERATE_HPP
#define CPPUTILS_FUNCTIONAL_ENUMERATE_HPP

#include "../traits/cpputils_concepts.hpp"
#include "internal/adaptors.hpp"
#include "internal/iter_utils.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>


namespace cpputils {

template <std::ranges::input_range Range,
          std::weakly_incrementable Index = std::size_t>
// clang-format off
requires std::copyable<Index> &&
         std::is_default_constructible_v<Index> &&
         std::ranges::view<Range>
// clang-format on
class enumerate_view : public std::ranges::view_interface<enumerate_view<Range, Index>> {
public:
    struct enumeration {
        Index index;
        std::ranges::range_reference_t<Range> value;
    };

    constexpr enumerate_view() = default;

    constexpr explicit enumerate_view(Range rng, Index start)
        : m_iter{rng}
        , m_start{start} {}

    constexpr explicit enumerate_view(Range rng)
        : enumerate_view{rng, std::size_t{0U}} {}

    class iterator {
        using iter_type = std::ranges::iterator_t<Range>;

    public:
        struct sentinel {};
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = enumeration;
        using pointer = void;
        using reference = value_type;

        constexpr explicit iterator() = default;

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

        // constexpr auto operator++(int) {  // NOLINT(cert-dcl21-cpp)
        //     auto it = *this;
        //     ++(*this);
        //     return it;
        // }

        constexpr void operator++(int) { ++*this; }

        constexpr bool operator==(iterator const &it) const {
            return m_iter == it.m_iter;
        }

        constexpr bool operator==(sentinel) const { return m_iter == m_last; }

    private:
        iter_type m_iter{};
        iter_type m_last{};
        Index m_index{};
    };

    constexpr auto begin() { return iterator{m_iter, m_start}; }
    constexpr auto end() { return typename iterator::sentinel{}; }

private:
    Range m_iter;
    Index m_start;
};

template <typename Range, typename Index>
enumerate_view(Range &&, Index &&)
    -> enumerate_view<std::ranges::views::all_t<Range>, std::remove_cvref_t<Index>>;

inline constexpr auto enumerate = detail::adaptors::range_adaptor{
    [](std::ranges::input_range auto &&r, auto &&i = std::size_t{0U}) {
        return enumerate_view{std::forward<decltype(r)>(r), std::forward<decltype(i)>(i)};
    }  // namespace cpputils
};

}  // namespace cpputils

#endif
