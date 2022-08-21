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
    concept concept_name = is_##trait_name##_v<Q> /* NOLINT */

// NOLINTNEXTLINE
#define CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(result_quantity, lhs_quantity, operator_type, rhs_quantity)                      \
    [[nodiscard]] constexpr result_quantity operator operator_type(lhs_quantity const &lhs, rhs_quantity const &rhs) { /* NOLINT */ \
        return result_quantity{static_cast<result_quantity::value_type>(lhs.value() operator_type rhs.value())}; /* NOLINT */       \
    }

// NOLINTNEXTLINE
#define CPPUTILS_DEFINE_DERIVED_QUANTITY_SIMMETRIC_OPERATION(result_quantity, lhs_quantity, operator_type, rhs_quantity)            \
    CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(result_quantity, lhs_quantity, operator_type, hs_quantity)                           \
    [[nodiscard]] constexpr result_quantity operator operator_type(rhs_quantity const &rhs, lhs_quantity const &lhs) { /* NOLINT */ \
        return result_quantity{static_cast<result_quantity::value_type>(rhs.value() operator_type lhs.value())}; /* NOLINT */       \
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


template <auto ToBase, auto FromBase>
using temperature = quantity<FromBase, ToBase, f64, quantity_tags::Temp_Tag>;

using kelvin = temperature<std::identity{}, std::identity{}>;

using celsius = temperature<[](f64 celsius_) -> f64 { return celsius_ + detail::g_kelvin_reference; },
                            [](f64 kelvin_) -> f64 { return kelvin_ - detail::g_kelvin_reference; }>;

using fahrenheit = temperature<[](f64 fah) { return (fah - f64{32.0}) * f64{5.0 / 9.0} + detail::g_kelvin_reference; },  // NOLINT
                               [](f64 kel) { return (kel - detail::g_kelvin_reference) * f64{9.0 / 5.0} + f64{32.0}; }>;  // NOLINT


template <auto ToBase, auto FromBase>
using length = quantity<ToBase, FromBase, f64, quantity_tags::Length_Tag>;

using meter = length<std::identity{}, std::identity{}>;

using kilometer = length<[](f64 km) { return km * f64{1000.0}; },  // NOLINT
                         [](f64 m) { return m / f64{1000.0}; }>;  // NOLINT

using mile = length<[](f64 mi) { return mi * detail::g_mile_reference; },
                    [](f64 m) { return m / detail::g_mile_reference; }>;


template <auto ToBase, auto FromBase>
using time = quantity<ToBase, FromBase, f64, quantity_tags::Time_Tag>;

using milliseconds = time<[](f64 ms) { return ms / f64{1000.}; }, [](f64 sec) { return sec * f64{1000.}; }>;  // NOLINT
using seconds = time<std::identity{}, std::identity{}>;
using minutes = time<[](f64 min) { return min * f64{60}; }, [](f64 sec) { return sec / f64{60}; }>;  // NOLINT
using hours = time<[](f64 hrs) { return hrs * f64{60 * 60}; }, [](f64 sec) { return sec / f64{60 * 60}; }>;  // NOLINT

template <auto ToBase, auto FromBase>
using velocity = quantity<ToBase, FromBase, f64, quantity_tags::Velocity_Tag>;

using meters_per_second = velocity<std::identity{}, std::identity{}>;
using kilometers_per_hour = velocity<[](f64 kmph) { return kmph / f64{3.6}; }, [](f64 mps) { return mps * f64{3.6}; }>;  // NOLINT


CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(meters_per_second, meter, *, seconds)
CPPUTILS_DEFINE_DERIVED_QUANTITY_OPERATION(kilometers_per_hour, kilometer, *, hours)

namespace literals {
    CPPULTILS_DEFINE_QUANTITY_LITERAL(kelvin, _k, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(celsius, _deg, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(fahrenheit, _fah, long double)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(kelvin, _k, unsigned long long int)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(celsius, _deg, unsigned long long int)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(fahrenheit, _fah, unsigned long long int)


    CPPULTILS_DEFINE_QUANTITY_LITERAL(meter, _m, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometer, _km, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(mile, _mi, long double)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(meter, _m, unsigned long long int)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometer, _km, unsigned long long int)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(mile, _mi, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds, _msec, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds, _msec, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds, _sec, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(seconds, _sec, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(minutes, _min, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(minutes, _min, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(hours, _hrs, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(hours, _hrs, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(meters_per_second, _mps, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(meters_per_second, _mps, unsigned long long int)

    CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometers_per_hour, _kmph, long double)
    CPPULTILS_DEFINE_QUANTITY_LITERAL(kilometers_per_hour, _kmph, unsigned long long int)
}  // namespace literals

template <typename Time>
[[nodiscard]] constexpr auto as_chrono(Time t) {
    if constexpr (std::is_same_v<Time, milliseconds>) {
        return std::chrono::milliseconds{static_cast<std::chrono::milliseconds::rep>(t.value())};
    } else if constexpr (std::is_same_v<Time, seconds>) {
        return std::chrono::seconds{static_cast<std::chrono::seconds::rep>(t.value())};
    } else if constexpr (std::is_same_v<Time, minutes>) {
        return std::chrono::minutes{static_cast<std::chrono::minutes::rep>(t.value())};
    } else if constexpr (std::is_same_v<Time, hours>) {
        return std::chrono::hours{static_cast<std::chrono::hours::rep>(t.value())};
    }
}

}  // namespace cpputils

#undef CPPUTILS_DEFINE_QUANTITY_TRAITS
#endif
