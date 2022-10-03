#ifndef CPPUTILS_QUANTITIES_HPP
#define CPPUTILS_QUANTITIES_HPP

#include "quantity.hpp"
#include <chrono>

namespace cpputils {
// NOLINTNEXTLINE
#define CPPUTILS_DEFINE_QUANTITY_TRAITS(trait_name, concept_name, tag)           \
    template <typename>                                                          \
    struct is_##trait_name : std::false_type {};                                 \
                                                                                 \
    template <auto ToBase, auto FromBase, typename UnderlyingType, typename Tag> \
    struct is_##trait_name<quantity<ToBase, FromBase, UnderlyingType, Tag>>      \
        : std::conditional_t<std::is_same_v<Tag, tag>,                           \
                             std::true_type,                                     \
                             std::false_type> {};                                \
                                                                                 \
    template <typename Q>                                                        \
    inline constexpr auto is_##trait_name##_v = is_##trait_name<Q>::value;       \
                                                                                 \
    template <typename Q>                                                        \
    concept concept_name = is_                                                   \
    ##trait_name##_v<Q> /* NOLINT */

// NOLINTNEXTLINE
#define CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(result_quantity, lhs_quantity, operator_type, rhs_quantity)                                        \
    template <typename Unit>                                                                                                                          \
    [[nodiscard]] constexpr result_quantity<Unit> operator operator_type(lhs_quantity<Unit> const &lhs, rhs_quantity<Unit> const &rhs) { /* NOLINT */ \
        return result_quantity<Unit>{static_cast<result_quantity<Unit>::value_type>(lhs.value() operator_type rhs.value())}; /* NOLINT */             \
    }

// NOLINTNEXTLINE
#define CPPUTILS_DEFINE_DERIVED_QUANTITY_SIMMETRIC_OPERATION(result_quantity, lhs_quantity, operator_type, rhs_quantity)                              \
    CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(result_quantity, lhs_quantity, operator_type, hs_quantity)                                             \
    template <typename Unit>                                                                                                                          \
    [[nodiscard]] constexpr result_quantity<Unit> operator operator_type(rhs_quantity<Unit> const &rhs, lhs_quantity<Unit> const &lhs) { /* NOLINT */ \
        return result_quantity<Unit>{static_cast<result_quantity<Unit>::value_type>(rhs.value() operator_type lhs.value())}; /* NOLINT */             \
    }


// NOLINTNEXTLINE
#define CPPULTILS_DEFINE_QUANTITY_LITERAL(q_name, literal, input_type)  \
    consteval q_name operator"" literal(input_type i) { /* NOLINT */    \
        return q_name{static_cast<q_name::value_type>(i)}; /* NOLINT */ \
    }

CPPUTILS_DEFINE_QUANTITY_TRAITS(temperature, Temperature, quantity_tags::Temp_Tag);
CPPUTILS_DEFINE_QUANTITY_TRAITS(length, Length, quantity_tags::Length_Tag);
CPPUTILS_DEFINE_QUANTITY_TRAITS(time, Time, quantity_tags::Time_Tag);
CPPUTILS_DEFINE_QUANTITY_TRAITS(velocity, Velocity, quantity_tags::Velocity_Tag);

namespace detail {
    template <typename Unit>
    inline constexpr auto g_kelvin_reference = static_cast<Unit>(273.15);

    template <typename Unit>
    inline constexpr auto g_mile_reference = static_cast<Unit>(1609.344);

    inline constexpr auto celsius_to_kelvin = []<typename Unit>(Unit celsius_) -> Unit { return celsius_ + g_kelvin_reference<Unit>; };
    inline constexpr auto kelvin_to_celsius = []<typename Unit>(Unit kelvin_) -> Unit { return kelvin_ - g_kelvin_reference<Unit>; };

    inline constexpr auto fahrenheit_to_kelvin = []<typename Unit>(Unit fah) { return (fah - static_cast<Unit>(32)) * static_cast<Unit>(5.0 / 9.0) + g_kelvin_reference<Unit>; };  // NOLINT
    inline constexpr auto kelvin_to_fahrenheit = []<typename Unit>(Unit kel) { return (kel - g_kelvin_reference<Unit>)*static_cast<Unit>(9.0 / 5.0) + static_cast<Unit>(32); };  // NOLINT
}  // namespace detail

template <auto ToBase, auto FromBase, typename Unit>
using temperature = quantity<ToBase, FromBase, Unit, quantity_tags::Temp_Tag>;

template <typename Unit = f64>
using kelvin = temperature<std::identity{}, std::identity{}, Unit>;

template <typename Unit = f64>
using celsius = temperature<detail::celsius_to_kelvin, detail::kelvin_to_celsius, Unit>;

template <typename Unit = f64>
using fahrenheit = temperature<detail::fahrenheit_to_kelvin, detail::kelvin_to_fahrenheit, Unit>;


template <auto ToBase, auto FromBase, typename Unit>
using length = quantity<ToBase, FromBase, Unit, quantity_tags::Length_Tag>;

template <typename Unit = f64>
using meter = length<std::identity{}, std::identity{}, Unit>;

template <typename Unit = f64>
using kilometer = length<[](Unit km) { return km * static_cast<Unit>(1000); },  // NOLINT
                         [](Unit m) { return m / static_cast<Unit>(1000); },  // NOLINT
                         Unit>;

template <typename Unit = f64>
using mile = length<[](Unit mi) { return mi * detail::g_mile_reference<Unit>; },
                    [](Unit m) { return m / detail::g_mile_reference<Unit>; },
                    Unit>;


template <auto ToBase, auto FromBase, typename Unit>
using time = quantity<ToBase, FromBase, Unit, quantity_tags::Time_Tag>;

template <typename Unit = f64>
using milliseconds = time<[](Unit ms) { return ms / static_cast<Unit>(1000); }, [](Unit sec) { return sec * static_cast<Unit>(1000); }, Unit>;  // NOLINT

template <typename Unit = f64>
using seconds = time<std::identity{}, std::identity{}, Unit>;

template <typename Unit = f64>
using minutes = time<[](Unit min) { return min * static_cast<Unit>(60); }, [](Unit sec) { return sec / static_cast<Unit>(60); }, Unit>;  // NOLINT

template <typename Unit = f64>
using hours = time<[](Unit hrs) { return hrs * static_cast<Unit>(60 * 60); }, [](Unit sec) { return sec / static_cast<Unit>(60 * 60); }, Unit>;  // NOLINT

template <auto ToBase, auto FromBase, typename Unit>
using velocity = quantity<ToBase, FromBase, Unit, quantity_tags::Velocity_Tag>;

template <typename Unit = f64>
using meters_per_second = velocity<std::identity{}, std::identity{}, Unit>;

template <typename Unit = f64>
using kilometers_per_hour = velocity<[](Unit kmph) { return kmph / static_cast<Unit>(3.6); }, [](Unit mps) { return mps * static_cast<Unit>(3.6); }, Unit>;  // NOLINT


// CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(meters_per_second, meter, /, seconds)

// CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(kilometers_per_hour, kilometer, /, hours)


namespace literals {
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kelvin<f64>, _k, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(celsius<f64>, _deg, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(fahrenheit<f64>, _fah, long double)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kelvin<f64>, _k, unsigned long long int)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(celsius<f64>, _deg, unsigned long long int)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(fahrenheit<f64>, _fah, unsigned long long int)


    // CPPULTILS_DEFINE_QUANTITY_LITERAL(meter<f64>, _m, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometer<f64>, _km, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(mile<f64>, _mi, long double)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(meter<f64>, _m, unsigned long long int)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometer<f64>, _km, unsigned long long int)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(mile<f64>, _mi, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds<f64>, _msec, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds<f64>, _msec, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds<f64>, _sec, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds<f64>, _sec, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(minutes<f64>, _min, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(minutes<f64>, _min, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(hours<f64>, _hrs, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(hours<f64>, _hrs, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(meters_per_second<f64>, _mps, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(meters_per_second<f64>, _mps, unsigned long long int)

    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometers_per_hour<f64>, _kmph, long double)
    // CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometers_per_hour<f64>, _kmph, unsigned long long int)
}  // namespace literals

template <auto ToBase, auto FromBase, typename Unit>
[[nodiscard]] constexpr auto as_chrono(time<ToBase, FromBase, Unit> t) {
    using time_type = time<ToBase, FromBase, Unit>;
    if constexpr (std::is_same_v<time_type, milliseconds<Unit>>) {
        return std::chrono::milliseconds{static_cast<std::chrono::milliseconds::rep>(t.value())};
    } else if constexpr (std::is_same_v<time_type, seconds<Unit>>) {
        return std::chrono::seconds{static_cast<std::chrono::seconds::rep>(t.value())};
    } else if constexpr (std::is_same_v<time_type, minutes<Unit>>) {
        return std::chrono::minutes{static_cast<std::chrono::minutes::rep>(t.value())};
    } else if constexpr (std::is_same_v<time_type, hours<Unit>>) {
        return std::chrono::hours{static_cast<std::chrono::hours::rep>(t.value())};
    }
}

}  // namespace cpputils

#undef CPPUTILS_DEFINE_QUANTITY_TRAITS
#endif
