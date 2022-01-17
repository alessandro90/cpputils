#ifndef CPPUTILS_STRONG_NUMBER_HPP
#define CPPUTILS_STRONG_NUMBER_HPP

#include "../traits/cpputils_concepts.hpp"
#include "../traits/is_specialization_of.hpp"
#include <cassert>
#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <ostream>
#include <type_traits>
#include <utility>


// TODO:
// - wrapping for signed numbers

// NOLINTNEXTLINE
#define MAX_N(underlying_type) std::numeric_limits<underlying_type>::max()
// NOLINTNEXTLINE
#define MIN_N(underlying_type) std::numeric_limits<underlying_type>::min()

namespace cpputils {
namespace detail {
    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;
}

template <detail::arithmetic>
class number;

namespace detail {
    // Utility to get the underlying numeric type
    template <typename>
    struct inner_type;

    template <typename I>
    struct inner_type<number<I>> {
        using type = I;
    };

    template <typename I>
    using inner_type_t = typename inner_type<I>::type;

    template <typename T>
    using is_number = typename is<T>::template specialization_of<number>;

    template <typename T>
    inline constexpr bool is_number_v = is_number<T>::value;

    template <typename T>
    concept a_modulo = std::numeric_limits<T>::is_modulo;

    template <typename T>
    concept a_bit_ops_compatible = std::unsigned_integral<T> && !std::is_same_v<T, bool>;

    template <typename T>
    concept convertible = arithmetic<T> || an<T, number>;

    template <typename underlying_type>
    consteval void assert_valid_range(std::integral auto v) {
        assert(std::cmp_greater_equal(v, MIN_N(underlying_type))
               && std::cmp_less_equal(v, MAX_N(underlying_type)));
    }

    template <typename underlying_type>
    consteval void assert_valid_range(std::floating_point auto v) {
        assert(v >= MIN_N(underlying_type) && v <= MAX_N(underlying_type));
    }

    template <typename I>
    struct number_common {
    public:
        explicit constexpr number_common() noexcept = default;
        explicit constexpr number_common(I i) noexcept
            : m_i{i} {}

        [[nodiscard]] constexpr auto primitive() const noexcept { return m_i; }

        template <convertible NewType>
        [[nodiscard]] constexpr auto as() const noexcept {
            if constexpr (std::is_integral_v<NewType> || std::is_floating_point_v<NewType>) {
                return static_cast<NewType>(m_i);
            } else {
                return NewType{static_cast<detail::inner_type_t<NewType>>(m_i)};
            }
        }

    protected:
        I m_i;
    };
}  // namespace detail

template <detail::arithmetic I>
class number : private detail::number_common<I> {
public:
    using detail::number_common<I>::primitive;
    using detail::number_common<I>::as;

    [[nodiscard]] inline static constexpr auto max() noexcept {
        return number{std::numeric_limits<I>::max()};
    };

    [[nodiscard]] static constexpr auto min() noexcept {
        return number{std::numeric_limits<I>::min()};
    };

    explicit constexpr number() noexcept = default;
    explicit constexpr number(I i) noexcept
        : detail::number_common<I>{i} {}

    template <detail::convertible NewType>
    [[nodiscard]] explicit constexpr operator NewType() const noexcept { return this->template as<NewType>(); }

    [[nodiscard]] constexpr auto operator<=>(number const &) const noexcept requires std::integral<I>
    = default;

    [[nodiscard]] constexpr auto operator<(number const &rhs) const noexcept requires std::floating_point<I> {
        return this->m_i < rhs.m_i;
    }

    [[nodiscard]] constexpr auto operator>(number const &rhs) const noexcept requires std::floating_point<I> {
        return this->m_i > rhs.m_i;
    }

    friend std::ostream &operator<<(std::ostream &os, number const &n) {
        return os << n.m_i;
    }

    // wrapping ops
    [[nodiscard]] constexpr auto wrapping_add(number const &rhs) const noexcept requires detail::a_modulo<I> {
        auto temp{*this};
        temp.m_i += rhs.m_i;
        return temp;
    }

    [[nodiscard]] constexpr auto wrapping_sub(number const &rhs) const noexcept requires detail::a_modulo<I> {
        auto temp{*this};
        temp.m_i -= rhs.m_i;
        return temp;
    }

    // Modulo ops
    constexpr auto &operator%=(number const &rhs) noexcept requires std::integral<I> {
        this->m_i %= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator%(number const &rhs) noexcept requires std::integral<I> {
        return number{static_cast<I>(this->m_i % rhs.m_i)};
    }

    // Common maths
    constexpr auto &operator++() noexcept {
        assert(this->m_i < max().primitive());
        ++this->m_i;
        return *this;
    }

    constexpr auto operator++(int) noexcept {
        assert(this->m_i < max().primitive());
        auto const temp{*this};
        ++this->m_i;
        return temp;
    }

    constexpr auto &operator--() noexcept {
        assert(this->m_i > min().primitive());
        --this->m_i;
        return *this;
    }

    constexpr auto operator--(int) noexcept {
        assert(this->m_i > min().primitive());
        auto const temp{*this};
        --this->m_i;
        return temp;
    }

    constexpr auto &operator+=(number const &rhs) noexcept {
        assert(this->m_i <= max().primitive() - rhs.m_i);
        this->m_i += rhs.m_i;
        return *this;
    }

    constexpr auto &operator-=(number const &rhs) noexcept {
        assert(this->m_i >= min().primitive() + rhs.m_i);
        this->m_i -= rhs.m_i;
        return *this;
    }

    constexpr auto &operator*=(number const &rhs) noexcept {
        assert(this->m_i <= max().primitive() / rhs.m_i);
        this->m_i *= rhs.m_i;
        return *this;
    }

    constexpr auto &operator/=(number const &rhs) noexcept {
        this->m_i /= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const noexcept requires std::is_signed_v<I> {
        return number{static_cast<I>(-this->m_i)};
    }

    [[nodiscard]] constexpr auto operator+() const noexcept { return *this; }

    [[nodiscard]] constexpr auto operator+(number const &rhs) const noexcept {
        assert(this->m_i <= max().primitive() - rhs.m_i);
        return number{static_cast<I>(this->m_i + rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator-(number const &rhs) const noexcept {
        assert(this->m_i >= min().primitive() + rhs.m_i);
        return number{static_cast<I>(this->m_i - rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator*(number const &rhs) const noexcept {
        assert(this->m_i <= max().primitive().primitive / rhs.m_i);
        return number{static_cast<I>(this->m_i * rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator/(number const &rhs) const noexcept {
        return number{static_cast<I>(this->m_i / rhs.m_i)};
    }

    // bits operations
    constexpr auto &operator&=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        this->m_i &= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator&(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(this->m_i & rhs.m_i)};
    }

    constexpr auto &operator|=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        this->m_i |= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator|(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(this->m_i | rhs.m_i)};
    }

    constexpr auto &operator^=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        this->m_i ^= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator^(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(this->m_i ^ rhs.m_i)};
    }

    constexpr auto &operator<<=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        this->m_i <<= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator<<(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(this->m_i << rhs.m_i)};
    }

    constexpr auto &operator>>=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        this->m_i >>= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator>>(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(this->m_i >> rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator~() const noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(~this->m_i)};
    }
};

template <>
class number<bool> : private detail::number_common<bool> {
public:
    using detail::number_common<bool>::primitive;
    using detail::number_common<bool>::as;

    constexpr number() noexcept = default;
    explicit constexpr number(bool i) noexcept
        : detail::number_common<bool>{i} {}

    [[nodiscard]] explicit constexpr operator bool() const noexcept { return this->m_i; }

    [[nodiscard]] constexpr auto operator!() const noexcept { return number{!this->m_i}; }

    template <detail::convertible NewType>
    [[nodiscard]] explicit constexpr operator NewType() const noexcept { return as<NewType>(); }

    friend std::ostream &operator<<(std::ostream &os, number const &n) {
        return os << (n.m_i ? "TRUE" : "FALSE");
    }
};

// Aliases
using u8 = number<std::uint8_t>;
using u16 = number<std::uint16_t>;
using u32 = number<std::uint32_t>;
using u64 = number<std::uint64_t>;
using i8 = number<std::int8_t>;
using i16 = number<std::int16_t>;
using i32 = number<std::int32_t>;
using i64 = number<std::int64_t>;
using isize = number<std::ptrdiff_t>;
using usize = number<std::size_t>;
using f32 = number<float>;
using f64 = number<double>;
using boolean = number<bool>;

// Literals

// NOLINTNEXTLINE
#define NUMBER_LITERAL_GENERIC(name, arg_type)                     \
    [[nodiscard]] consteval name operator""_##name(arg_type v) {   \
        detail::assert_valid_range<detail::inner_type_t<name>>(v); \
        return name{static_cast<detail::inner_type_t<name>>(v)};   \
    }

// NOLINTNEXTLINE
#define NUMBER_LITERAL_OPERATOR_INTEGRAL(name) NUMBER_LITERAL_GENERIC(name, unsigned long long)

// NOLINTNEXTLINE
#define NUMBER_LITERAL_OPERATOR_FLOATING(name) NUMBER_LITERAL_GENERIC(name, long double)


NUMBER_LITERAL_OPERATOR_INTEGRAL(u8)
NUMBER_LITERAL_OPERATOR_INTEGRAL(u16)
NUMBER_LITERAL_OPERATOR_INTEGRAL(u32)
NUMBER_LITERAL_OPERATOR_INTEGRAL(u64)
NUMBER_LITERAL_OPERATOR_INTEGRAL(i8)
NUMBER_LITERAL_OPERATOR_INTEGRAL(i16)
NUMBER_LITERAL_OPERATOR_INTEGRAL(i32)
NUMBER_LITERAL_OPERATOR_INTEGRAL(i64)
NUMBER_LITERAL_OPERATOR_INTEGRAL(isize)
NUMBER_LITERAL_OPERATOR_INTEGRAL(usize)
NUMBER_LITERAL_OPERATOR_FLOATING(f32)
NUMBER_LITERAL_OPERATOR_FLOATING(f64)

inline constexpr auto True = boolean{true};
inline constexpr auto False = boolean{false};

#undef NUMBER_LITERAL_OPERATOR_INTEGRAL
#undef NUMBER_LITERAL_OPERATOR_FLOATING
#undef NUMBER_LITERAL_GENERIC
#undef MAX_N
#undef MIN_N
}  // namespace cpputils

namespace std {
template <::cpputils::detail::arithmetic T>
struct hash<::cpputils::number<T>> {
    constexpr ::std::size_t operator()(::cpputils::number<T> const &n) const noexcept {
        return ::std::hash<::std::remove_const_t<T>>()(n.primitive());
    }
};
}  // namespace std
#endif
