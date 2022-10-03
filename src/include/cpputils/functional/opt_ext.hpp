#ifndef CPPUTILS_OPTIONAL_EXTENSION_HPP
#define CPPUTILS_OPTIONAL_EXTENSION_HPP

#include "../meta/traits.hpp"
#include "expected.hpp"
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>


// Utility macros. #undef'd at end of file
// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

// NOLINTNEXTLINE
#define INVK(f, args) std::invoke(FWD(f), FWD(args))
// NOLINTNEXTLINE
#define INVK0(f) std::invoke(FWD(f))
// NOLINTNEXTLINE
#define INVKs(f, args) std::invoke(FWD(f), FWD(args)...)
// NOLINTNEXTLINE
#define RES_T(f, arg) std::invoke_result_t<decltype(f), decltype(arg)>
// NOLINTNEXTLINE
#define RES_T0(f) std::invoke_result_t<decltype(f)>
// NOLINTNEXTLINE
#define RES_Ts(f, args) std::invoke_result_t<decltype(f), decltype(args)...>
// NOLINTNEXTLINE
#define INVOCABLE(f, arg) std::invocable<decltype(f), decltype(arg)>
// NOLINTNEXTLINE
#define INVOCABLEs(f, args) std::invocable<decltype(f), decltype(args)...>


namespace cpputils {
namespace detail {
    template <typename T>
    concept optional_like = an<T, std::optional> || an<T, expected>;

    template <typename T>
    using reference_or_value = std::conditional_t<std::is_rvalue_reference_v<T>, std::remove_cvref_t<T>, T>;

    template <typename F>
    struct optional_adaptor_closure;

    template <typename F>
    struct optional_adaptor {
        F f;
        inline constexpr decltype(auto) operator()(auto &&...args) const {
            if constexpr (std::is_invocable_v<decltype(f), decltype(args)...>) {
                return std::invoke(f, FWD(args)...);
            } else {
                return optional_adaptor_closure{
                    [... args_ = FWD(args), f_ = f](optional_like auto &&opt) -> decltype(auto) {
                        return std::invoke(f_, FWD(opt), args_...);
                    }};
            }
        }
    };

    template <typename Callable>
    optional_adaptor(Callable) -> optional_adaptor<Callable>;

    template <typename F>
    struct optional_adaptor_closure : optional_adaptor<F> {
        inline friend constexpr decltype(auto) operator>>(optional_like auto &&opt, optional_adaptor_closure const &cls) {
            return std::invoke(cls.f, FWD(opt));
        }
    };

    template <typename Callable>
    optional_adaptor_closure(Callable) -> optional_adaptor_closure<Callable>;
}  // namespace detail

[[nodiscard]] inline constexpr auto map(auto &&f, detail::optional_like auto &&...opts) requires INVOCABLEs(f, *opts)
{
    using ret_t = RES_Ts(f, *opts);

    auto const ok = (opts && ...);
    if constexpr (is_specialization_v<ret_t, std::optional>) {
        return ok ? INVKs(f, *opts) : ret_t{};
    } else {
        return ok ? std::optional{INVKs(f, *opts)} : std::optional<ret_t>{};
    }
}

[[nodiscard]] inline constexpr auto try_or_else(auto &&f, std::invocable<> auto &&otherwise, detail::optional_like auto &&...opts)
    requires (INVOCABLEs(f, *opts) && std::same_as<RES_Ts(f, *opts), RES_T0(otherwise)>)
{
    return (opts && ...) ? INVKs(f, *opts) : INVK0(otherwise);
}

[[nodiscard]] inline constexpr auto try_or(auto &&f, auto &&otherwise, detail::optional_like auto &&...opts)
    requires (INVOCABLEs(f, *opts) && std::same_as<RES_Ts(f, *opts), std::remove_cvref_t<decltype(otherwise)>>)
{
    return (opts && ...) ? INVKs(f, *opts) : FWD(otherwise);
}

inline constexpr void apply(auto &&f, detail::optional_like auto &&...opts) requires INVOCABLEs(f, *opts)
{
    if ((opts && ...)) {
        (std::invoke((f), FWD(*opts)), ...);
    }
}

inline constexpr void apply_or_else(auto &&f, auto &&otherwise, detail::optional_like auto &&...opts)
    requires INVOCABLEs(f, *opts)
{
    if ((opts && ...)) {
        (std::invoke(f, FWD(*opts)), ...);
    } else {
        INVK0(otherwise);
    }
}

inline constexpr auto zip(detail::optional_like auto &&...opts) {
    using Tuple_t = std::tuple<detail::reference_or_value<decltype(FWD(opts).value())>...>;
    using Ret_t = std::optional<Tuple_t>;
    if ((opts && ...)) {
        return std::optional{Tuple_t{FWD(opts).value()...}};
    } else {
        return Ret_t{};
    }
}

inline constexpr auto or_else = detail::optional_adaptor{
    [](detail::optional_like auto &&opt, std::invocable<> auto &&f) -> decltype(auto) {
        if (!opt) { std::invoke(f); }
        return FWD(opt);
    }};
inline constexpr auto transform = detail::optional_adaptor{
    [](detail::optional_like auto &&opt, std::invocable<std::remove_cvref_t<decltype(FWD(opt).value())>> auto &&f) -> decltype(auto) {
        return map(FWD(f), FWD(opt));
    }};
inline constexpr auto if_value = detail::optional_adaptor{
    [](detail::optional_like auto &&opt, std::invocable<decltype(*opt)> auto &&f) -> decltype(auto) {
        if (opt) { std::invoke(f, *opt); }
        return FWD(opt);
    }};
inline constexpr auto unwrap = detail::optional_adaptor{
    [](detail::optional_like auto &&opt) -> decltype(auto) {
        return FWD(opt).value();
    }};
// clang-format off
inline constexpr auto unwrap_or = detail::optional_adaptor{
    [](detail::optional_like auto &&opt, auto &&else_) requires std::same_as<std::remove_cvref_t<decltype(FWD(opt).value())>, 
                                                                         std::remove_cvref_t<decltype(else_)>> {
        return FWD(opt).value_or(FWD(else_));
    }};
// clang-format on
// clang-format off
inline constexpr auto unwrap_or_else = detail::optional_adaptor{
    [](detail::optional_like auto &&opt, std::invocable<> auto &&else_) requires std::same_as<RES_T0(else_),
                                                                                          std::remove_cvref_t<decltype(FWD(opt).value())>> {
        if (opt) { return FWD(opt).value(); }
        return std::invoke(FWD(else_));
    }};
// clang-format on

inline constexpr auto to_optional = detail::optional_adaptor{
    [](auto &&arg) -> decltype(auto) {
        if constexpr (is_specialization_v<decltype(arg), std::optional>) {
            return FWD(arg);
        } else if constexpr (is_specialization_v<decltype(arg), expected>) {
            // std::optional does not support references but expected does.
            // Fallback to reference_wrapper to this special case
            using value_t = typename std::remove_cvref_t<decltype(arg)>::value_t;
            using opt_t = std::conditional_t<std::is_reference_v<value_t>,
                                             std::reference_wrapper<std::remove_reference_t<value_t>>,
                                             value_t>;
            if (arg) {
                if constexpr (std::is_reference_v<value_t>) {
                    return std::optional<opt_t>{std::reference_wrapper{FWD(arg).value()}};
                } else {
                    return std::optional<opt_t>{FWD(arg).value()};
                }
            } else {
                return std::optional<opt_t>{};
            }
        } else {
            return std::optional{FWD(arg)};
        }
    }};
}  // namespace cpputils

#undef FWD
#undef INVK
#undef INVK0
#undef INVKs
#undef RES_T
#undef RES_T0
#undef RES_Ts
#undef INVOCABLE
#undef INVOCABLEs
#endif