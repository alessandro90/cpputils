#ifndef CPPUTILS_TRAITS_HPP
#define CPPUTILS_TRAITS_HPP

#include <chrono>
#include <concepts>
#include <optional>
#include <type_traits>


namespace cpputils {
namespace detail {
    template <typename, template <typename...> typename>
    struct is_specialization : public std::false_type {};

    template <typename... Ts, template <typename...> typename U>
    struct is_specialization<U<Ts...>, U> : public std::true_type {};
}  // namespace detail

template <typename T, template <typename...> typename U>
struct is_specialization
    : public detail::is_specialization<std::remove_cvref_t<T>, U> {};

template <typename T, template <typename...> typename U>
inline constexpr auto is_specialization_v = is_specialization<T, U>::value;

template <typename T, template <typename...> typename Target>
concept an = is_specialization_v<std::remove_cvref_t<T>, Target>;

namespace detail {
    template <typename T, typename Q>
    struct is_same_underlying_type
        : std::conditional_t<std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Q>>,
                             std::true_type,
                             std::false_type> {};

    template <typename T, typename Q>
    concept same_underlying_type = is_same_underlying_type<T, Q>::value;
}  // namespace detail

template <typename T>
concept minimal_incrementable =
    requires (T t) {
        { ++t } -> detail::same_underlying_type<T>;
    };

namespace tl {
    template <template <typename...> typename Target, typename... Args>
    concept specializable =
        requires () {
            typename Target<Args...>;
        };

    template <typename T>
    concept predicate =
        requires (T) {
            { T::value } -> std::convertible_to<bool>;
        };

    template <typename T>
    concept transformation =
        requires () {
            typename T::type;
        };
}  // namespace tl

template <typename>
struct is_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <typename D>
inline constexpr auto is_duration_v = is_duration<D>::value;

template <typename D>
concept duration = is_duration_v<D>;

template <typename F, typename R, typename... Args>
concept invocable_r = std::is_invocable_r_v<R, F, Args...>;

template <typename T>
concept pair_like =
    requires (T t) {
        t.first;
        t.second;
    }
    && std::is_member_object_pointer_v<std::remove_cvref_t<decltype(&std::remove_cvref_t<T>::first)>>
    && std::is_member_object_pointer_v<std::remove_cvref_t<decltype(&std::remove_cvref_t<T>::second)>>;

namespace detail {
    struct to_everything {
        template <typename T>
        constexpr explicit(false) operator T &&() const;  // NOLINT

        template <typename T>
        constexpr explicit(false) operator T &() const;  // NOLINT

        template <typename T>
        constexpr explicit(false) operator T const &() const;  // NOLINT
    };

    template <std::size_t Arity, std::size_t MaxArity, typename F, typename... Args>
    struct arity_count {
        inline static constexpr auto value =
            std::is_invocable_v<F, Args...>
                ? std::optional<std::size_t>{sizeof...(Args)}
                : arity_count<Arity + 1, MaxArity, F, Args..., to_everything>::value;
    };

    template <std::size_t MaxArity, typename F, typename... Args>
    struct arity_count<MaxArity, MaxArity, F, Args...> {
        inline static constexpr auto value =
            std::is_invocable_v<F, Args...>
                ? std::optional<std::size_t>{sizeof...(Args)}
                : std::optional<std::size_t>{};
    };

};  // namespace detail

inline constexpr std::size_t default_max_arity_detectable = 50;

template <typename F, std::size_t MaxArity = default_max_arity_detectable>
struct arity {
    inline static constexpr auto value = detail::arity_count<0, MaxArity, F>::value;
};

template <typename R, typename C, typename... Args, std::size_t MaxArity>
struct arity<R (C::*)(Args...), MaxArity> {
private:
    struct self {};

public:
    inline static constexpr auto value = detail::arity_count<0, MaxArity, R(self, Args...)>::value;
};

template <typename F, std::size_t MaxArity = default_max_arity_detectable>
inline constexpr auto arity_v = arity<F, MaxArity>::value;


template <typename T, typename K>
concept different_than = (!std::is_same_v<T, K>);

template <typename From, typename To>
concept explicitly_convertible_to =
    requires {
        static_cast<To>(std::declval<From>());
    };

template <typename T>
concept optional_like =
    requires (T opt) {
        { opt.value() } -> different_than<void>;
        { *opt } -> different_than<void>;
        { opt.has_value() } -> std::same_as<bool>;
        { opt.value_or(std::declval<std::remove_cvref_t<decltype(opt.value())>>()) }
          -> std::convertible_to<std::remove_cvref_t<decltype(opt.value())>>;
    }
    && explicitly_convertible_to<T, bool>
    && std::same_as<decltype(std::declval<T>().value()), decltype(*std::declval<T>())>
    && std::same_as<decltype(std::declval<T &>().value()), decltype(*std::declval<T &>())>;

}  // namespace cpputils


#endif
