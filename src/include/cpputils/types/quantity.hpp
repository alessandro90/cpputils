#ifndef CPPUTILS_QUANTITY_HPP
#define CPPUTILS_QUANTITY_HPP

#include "number.hpp"
#include <compare>
#include <functional>
#include <type_traits>


namespace cpputils {
template <auto ToBase, auto FromBase, typename UnderlyingType, typename Tag>
requires std::is_arithmetic_v<UnderlyingType> || detail::is_number_v<UnderlyingType>
class quantity;

namespace detail {
    // template <typename Q, typename ValueType>
    // class Operations {
    // public:
    //     [[nodiscard]] constexpr Q operator+(Q const &rhs) const {
    //         return Q{static_cast<ValueType>(this->self().value() + rhs.value())};
    //     }

    //     [[nodiscard]] constexpr Q operator-(Q const &rhs) const {
    //         return Q{static_cast<ValueType>(this->self().value() - rhs.value())};
    //     }

    //     [[nodiscard]] constexpr auto operator/(Q const &rhs) const {
    //         return this->self().value() / rhs.value();
    //     }

    //     [[nodiscard]] constexpr auto operator*(ValueType const &rhs) const {
    //         return Q{static_cast<ValueType>(this->self().value() * rhs)};
    //     }

    //     [[nodiscard]] constexpr auto operator/(ValueType const &rhs) const {
    //         return Q{static_cast<ValueType>(this->self().value() / rhs)};
    //     }

    //     [[nodiscard]] constexpr auto operator-() const {
    //         return Q{static_cast<ValueType>(-this->self().value())};
    //     }

    // private:
    //     [[nodiscard]] Q &self() { return static_cast<Q &>(*this); }
    //     [[nodiscard]] Q const &self() const { return static_cast<Q const &>(*this); }
    // };

    inline constexpr auto g_kelvin_reference = f64{273.15};
    inline constexpr auto g_mile_reference = f64{1609.344};

}  // namespace detail

namespace quantity_tags {
    struct Temp_Tag;
    struct Length_Tag;
    struct Velocity_Tag;
    struct Time_Tag;
}  // namespace quantity_tags

template <auto ToBase, auto FromBase, typename UnderlyingType, typename Tag>
requires std::is_arithmetic_v<UnderlyingType> || detail::is_number_v<UnderlyingType>
class quantity {
public:
    using value_type = UnderlyingType;

    explicit constexpr quantity(value_type v)
        : m_value{v} {
    }

    [[nodiscard]] constexpr quantity operator+(quantity const &rhs) const {
        return quantity{static_cast<value_type>(value() + rhs.value())};
    }

    [[nodiscard]] constexpr quantity operator-(quantity const &rhs) const {
        return quantity{static_cast<value_type>(value() - rhs.value())};
    }

    [[nodiscard]] constexpr auto operator/(quantity const &rhs) const {
        return value() / rhs.value();
    }

    [[nodiscard]] constexpr quantity operator*(value_type const &rhs) const {
        return quantity{static_cast<value_type>(value() * rhs)};
    }

    [[nodiscard]] constexpr quantity operator/(value_type const &rhs) const {
        return quantity{static_cast<value_type>(value() / rhs)};
    }

    [[nodiscard]] constexpr quantity operator-() const {
        return quantity{static_cast<value_type>(-value())};
    }

    [[nodiscard]] constexpr quantity operator+() const noexcept {
        return *this;
    }

    [[nodiscard]] constexpr value_type value() const noexcept { return m_value; }

    [[nodiscard]] constexpr auto operator<=>(const quantity &) const = default;

    template <auto ToBase_, auto FromBase_, typename UnderlyingType_>
    requires(!std::is_same_v<decltype(ToBase), decltype(ToBase_)> || !std::is_same_v<decltype(FromBase), decltype(FromBase_)> || !std::is_same_v<UnderlyingType, UnderlyingType_>)
        [[nodiscard]] constexpr auto
        operator<=>(quantity<ToBase_, FromBase_, UnderlyingType_, Tag> const &rhs) const {
        return value() <=> FromBase(ToBase_(rhs.value()));
    }

    template <auto ToBase_, auto FromBase_>
    explicit(false) constexpr operator quantity<ToBase_, FromBase_, UnderlyingType, Tag>() const {  // NOLINT
        return quantity<ToBase_, FromBase_, UnderlyingType, Tag>{FromBase_(ToBase(value()))};
    }

private:
    value_type m_value;
};
}  // namespace cpputils

#endif
