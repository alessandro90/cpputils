#ifndef CPPUTILS_LAZY_HPP
#define CPPUTILS_LAZY_HPP

#include <concepts>
#include <functional>
#include <utility>


// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {

template <typename Builder, typename Target>
concept builder_for =
    requires (Builder b) {
        { b() } -> std::same_as<Target>;
    };

template <typename T>
inline constexpr auto lazy = []<typename... Args>(Args && ...args) requires std::constructible_from<T, std::remove_cvref_t<Args>...>
{
    return [... args_ = FWD(args)]() -> T { return T{args_...}; };
};


template <typename T, builder_for<T> Builder = std::function<T()>>
class [[nodiscard]] Lazy {
public:
    template <typename... Args>
    requires (!std::disjunction_v<std::is_same<Lazy<T>, std::remove_cvref_t<Args>>...>)
    explicit constexpr Lazy(Args &&...args) requires std::constructible_from<T, std::remove_cvref_t<Args>...>
        : m_builder{[... args_ = FWD(args)]() -> T { return T{args_...}; }} {
    }

    [[nodiscard]] explicit constexpr operator T() const {
        return m_builder();
    }

    [[nodiscard]] constexpr T operator()() const { return m_builder(); }

private:
    Builder m_builder;
};

}  // namespace cpputils

#undef FWD

#endif
