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

// Policy for wrong computations
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
    using is_number = is_specialization<T, number>;

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
#ifndef NDEBUG
        assert(std::cmp_greater_equal(v, MIN_N(underlying_type))
               && std::cmp_less_equal(v, MAX_N(underlying_type)));
#else
        [[maybe_unused]] v;
#endif
    }

    template <typename underlying_type>
    consteval void assert_valid_range(std::floating_point auto v) {
#ifndef NDEBUG
        assert(v >= MIN_N(underlying_type) && v <= MAX_N(underlying_type));
#else
        [[maybe_unused]] v;
#endif
    }

    template <convertible NewType>
    [[nodiscard]] constexpr auto as(arithmetic auto v) noexcept {
        if constexpr (std::is_integral_v<NewType> || std::is_floating_point_v<NewType>) {
            return static_cast<NewType>(v);
        } else {
            return NewType{static_cast<detail::inner_type_t<NewType>>(v)};
        }
    }

    template <std::unsigned_integral I>
    constexpr bool is_error_result_add(I lhs, I rhs) noexcept {
        return lhs > MAX_N(I) - rhs;
    }
    template <std::unsigned_integral I>
    constexpr bool is_error_result_sub(I lhs, I rhs) noexcept {
        return rhs > 0 && lhs < rhs;
    }
    template <std::unsigned_integral I>
    constexpr bool is_error_result_mul(I lhs, I rhs) noexcept {
        return rhs == 0 ? false : lhs > MAX_N(I) / rhs;
    }
    template <std::unsigned_integral I>
    constexpr bool is_error_result_div(I, I rhs) noexcept {
        return rhs == 0;
    }

    template <std::signed_integral I>
    constexpr bool is_error_result_add(I lhs, I rhs) noexcept {
        if (lhs >= 0 && rhs >= 0) { return lhs > MAX_N(I) - rhs; }
        if (lhs < 0 && rhs < 0) { return lhs < MIN_N(I) - rhs; }
        return false;
    }

    template <std::signed_integral I>
    constexpr bool is_error_result_sub(I lhs, I rhs) noexcept {
        if (lhs >= 0 && rhs < 0) { return lhs > MAX_N(I) + rhs; }
        if (lhs < 0 && rhs >= 0) { return lhs < MIN_N(I) + rhs; }
        return false;
    }

    template <std::signed_integral I>
    constexpr bool is_error_result_mul(I lhs, I rhs) noexcept {
        if (rhs == 0 || lhs == 0) { return false; }
        if (lhs > 0 && rhs > 0) { return lhs > MAX_N(I) / rhs; }
        if (lhs < 0 && rhs < 0) { return lhs < MAX_N(I) / rhs; }
        return lhs < MIN_N(I) / rhs;
    }

    template <std::signed_integral I>
    constexpr bool is_error_result_div(I lhs, I rhs) noexcept {
        if (rhs == 0) { return true; }
        return lhs == MIN_N(I) && rhs == -1;
    }

    template <std::integral I>
    constexpr bool is_error_result_mod(I, I rhs) noexcept {
        return rhs == 0;
    }

    template <std::floating_point I>
    constexpr bool is_error_result_add(I, I) noexcept { return false; }
    template <std::floating_point I>
    constexpr bool is_error_result_sub(I, I) noexcept { return false; }
    template <std::floating_point I>
    constexpr bool is_error_result_mul(I, I) noexcept { return false; }
    template <std::floating_point I>
    constexpr bool is_error_result_div(I, I) noexcept { return false; }
}  // namespace detail

template <detail::arithmetic I>
class number {
public:
    explicit constexpr number() noexcept = default;
    explicit constexpr number(I i) noexcept
        : m_i{i} {}

    [[nodiscard]] constexpr auto val() const noexcept { return m_i; }

    template <detail::convertible NewType>
    [[nodiscard]] explicit constexpr operator NewType() const noexcept { return detail::as<NewType>(m_i); }

    [[nodiscard]] constexpr auto operator<=>(number const &) const noexcept requires std::integral<I>
    = default;


    [[nodiscard]] constexpr auto operator<(number const &rhs) const noexcept requires std::floating_point<I> {
        return m_i < rhs.m_i;
    }

    [[nodiscard]] constexpr auto operator>(number const &rhs) const noexcept requires std::floating_point<I> {
        return m_i > rhs.m_i;
    }

    friend std::ostream &operator<<(std::ostream &os, number const &n) {
        return os << n.m_i;
    }

    template <detail::convertible NewType>
    [[nodiscard]] constexpr auto as() const noexcept { return detail::as<NewType>(m_i); }

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
        assert(!detail::is_error_result_mod(m_i, rhs.m_i));
        m_i %= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator%(number const &rhs) noexcept requires std::integral<I> {
        assert(!detail::is_error_result_mod(m_i, rhs.m_i));
        return number{static_cast<I>(m_i % rhs.m_i)};
    }

    // Common maths
    constexpr auto &operator++() noexcept {
        assert(!detail::is_error_result_add(m_i, I{1}));
        ++m_i;
        return *this;
    }

    constexpr auto operator++(int) noexcept {  // NOLINT(cert-dcl21-cpp)
        assert(!detail::is_error_result_add(m_i, I{1}));
        auto const temp{*this};
        ++m_i;
        return temp;
    }

    constexpr auto &operator--() noexcept {
        assert(!detail::is_error_result_sub(m_i, I{1}));
        --m_i;
        return *this;
    }

    constexpr auto operator--(int) noexcept {  // NOLINT(cert-dcl21-cpp)
        assert(!detail::is_error_result_sub(m_i, I{1}));
        auto const temp{*this};
        --m_i;
        return temp;
    }

    constexpr auto &operator+=(number const &rhs) noexcept {
        assert(!detail::is_error_result_add(m_i, rhs.m_i));
        m_i += rhs.m_i;
        return *this;
    }

    constexpr auto &operator-=(number const &rhs) noexcept {
        assert(!detail::is_error_result_sub(m_i, rhs.m_i));
        m_i -= rhs.m_i;
        return *this;
    }

    constexpr auto &operator*=(number const &rhs) noexcept {
        assert(!detail::is_error_result_mul(m_i, rhs.m_i));
        m_i *= rhs.m_i;
        return *this;
    }

    constexpr auto &operator/=(number const &rhs) noexcept {
        assert(!detail::is_error_result_div(m_i, rhs.m_i));
        m_i /= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const noexcept requires std::is_signed_v<I> {
        assert(!detail::is_error_result_mul(m_i, I{-1}));
        return number{static_cast<I>(-m_i)};
    }

    [[nodiscard]] constexpr auto operator+() const noexcept { return *this; }

    [[nodiscard]] constexpr auto operator+(number const &rhs) const noexcept {
        assert(!detail::is_error_result_add(m_i, rhs.m_i));
        return number{static_cast<I>(m_i + rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator-(number const &rhs) const noexcept {
        assert(!detail::is_error_result_sub(m_i, rhs.m_i));
        return number{static_cast<I>(m_i - rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator*(number const &rhs) const noexcept {
        assert(!detail::is_error_result_mul(m_i, rhs.m_i));
        return number{static_cast<I>(m_i * rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator/(number const &rhs) const noexcept {
        assert(!detail::is_error_result_div(m_i, rhs.m_i));
        return number{static_cast<I>(m_i / rhs.m_i)};
    }

    // bits operations
    constexpr auto &operator&=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        m_i &= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator&(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(m_i & rhs.m_i)};
    }

    constexpr auto &operator|=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        m_i |= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator|(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(m_i | rhs.m_i)};
    }

    constexpr auto &operator^=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        m_i ^= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator^(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(m_i ^ rhs.m_i)};
    }

    constexpr auto &operator<<=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        m_i <<= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator<<(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(m_i << rhs.m_i)};
    }

    constexpr auto &operator>>=(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        m_i >>= rhs.m_i;
        return *this;
    }

    [[nodiscard]] constexpr auto operator>>(number const &rhs) noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(m_i >> rhs.m_i)};
    }

    [[nodiscard]] constexpr auto operator~() const noexcept requires detail::a_bit_ops_compatible<I> {
        return number{static_cast<I>(~m_i)};
    }
    // clang-format off
private:
    I m_i;
    // clang-format on
};

template <>
class number<bool> {
public:
    constexpr number() noexcept = default;
    explicit constexpr number(bool i) noexcept
        : m_i{i} {}

    [[nodiscard]] explicit constexpr operator bool() const noexcept { return m_i; }

    [[nodiscard]] constexpr auto operator!() const noexcept { return number{!m_i}; }

    template <detail::convertible NewType>
    [[nodiscard]] explicit constexpr operator NewType() const noexcept { return detail::as<NewType>(m_i); }

    template <detail::convertible NewType>
    [[nodiscard]] constexpr auto as() const noexcept { return detail::as<NewType>(m_i); }

    friend std::ostream &operator<<(std::ostream &os, number const &n) {
        return os << (n.m_i ? "TRUE" : "FALSE");
    }

private:
    bool m_i;
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
#define NUMBER_LITERAL_GENERIC(name, arg_type)                                  \
    [[nodiscard]] consteval name operator""_##name(arg_type v) { /* NOLINT */   \
        detail::assert_valid_range<detail::inner_type_t<name>>(v); /* NOLINT */ \
        return name{static_cast<detail::inner_type_t<name>>(v)}; /* NOLINT */   \
    }

// NOLINTNEXTLINE
#define NUMBER_LITERAL_OPERATOR_INTEGRAL(name) NUMBER_LITERAL_GENERIC(name, unsigned long long)

// NOLINTNEXTLINE
#define NUMBER_LITERAL_OPERATOR_FLOATING(name) NUMBER_LITERAL_GENERIC(name, long double)

namespace literals {
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
}  // namespace literals
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
        return ::std::hash<::std::remove_const_t<T>>()(n.val());
    }
};

template <typename I>
struct numeric_limits<::cpputils::number<I>> {
    inline static constexpr auto is_specialized = numeric_limits<I>::is_specialized;
    inline static constexpr auto is_signed = numeric_limits<I>::is_signed;
    inline static constexpr auto is_integer = numeric_limits<I>::is_integer;
    inline static constexpr auto is_exact = numeric_limits<I>::is_exact;
    inline static constexpr auto has_infinity = numeric_limits<I>::has_infinity;
    inline static constexpr auto has_quiet_NaN = numeric_limits<I>::has_quiet_NaN;
    inline static constexpr auto has_signaling_NaN = numeric_limits<I>::has_signaling_NaN;
    inline static constexpr auto has_denorm = numeric_limits<I>::has_denorm;
    inline static constexpr auto has_denorm_loss = numeric_limits<I>::has_denorm_loss;
    inline static constexpr auto round_style = numeric_limits<I>::round_style;
    inline static constexpr auto is_iec559 = numeric_limits<I>::is_iec559;
    inline static constexpr auto is_bounded = numeric_limits<I>::is_bounded;
    inline static constexpr auto is_modulo = numeric_limits<I>::is_modulo;
    inline static constexpr auto digits = numeric_limits<I>::digits;
    inline static constexpr auto digits10 = numeric_limits<I>::digits10;
    inline static constexpr auto max_digits10 = numeric_limits<I>::max_digits10;
    inline static constexpr auto radix = numeric_limits<I>::radix;
    inline static constexpr auto min_exponent = numeric_limits<I>::min_exponent;
    inline static constexpr auto min_exponent10 = numeric_limits<I>::min_exponent10;
    inline static constexpr auto max_exponent = numeric_limits<I>::max_exponent;
    inline static constexpr auto max_exponent10 = numeric_limits<I>::max_exponent10;
    inline static constexpr auto traps = numeric_limits<I>::traps;
    inline static constexpr auto tinyness_before = numeric_limits<I>::tinyness_before;

    static constexpr auto min() { return ::cpputils::number<I>{numeric_limits<I>::min()}; }
    static constexpr auto lowest() { return ::cpputils::number<I>{numeric_limits<I>::lowest()}; }
    static constexpr auto max() { return ::cpputils::number<I>{numeric_limits<I>::max()}; }
    static constexpr auto epsilon() { return ::cpputils::number<I>{numeric_limits<I>::epsilon()}; }
    static constexpr auto round_error() { return ::cpputils::number<I>{numeric_limits<I>::round_error()}; }
    static constexpr auto infinity() { return ::cpputils::number<I>{numeric_limits<I>::infinity()}; }
    static constexpr auto quiet_NaN() { return ::cpputils::number<I>{numeric_limits<I>::quiet_NaN()}; }
    static constexpr auto signaling_NaN() { return ::cpputils::number<I>{numeric_limits<I>::signaling_NaN()}; }
    static constexpr auto denorm_min() { return ::cpputils::number<I>{numeric_limits<I>::denorm_min()}; }
};
}  // namespace std
#endif
