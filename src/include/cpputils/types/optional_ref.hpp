#ifndef CPPUTILS_OPTIONAL_REFERENCE_HPP
#define CPPUTILS_OPTIONAL_REFERENCE_HPP

#include <cassert>
#include <compare>
#include <concepts>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>


namespace cpputils {

class bad_optional_ref_access : public std::bad_optional_access {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "optional_ref invalid access";
    }
};

template <typename T>
requires (!std::is_reference_v<T> || std::is_same_v<T, void>)
class optional_ref {
public:
    using value_type = T;

    constexpr optional_ref() = default;

    explicit constexpr optional_ref(T &val)
        : m_val(std::addressof(val)) {}

    explicit(false) constexpr optional_ref(std::nullopt_t)  // NOLINT
    {}

    explicit constexpr optional_ref(T &&val) = delete;

    [[nodiscard]] constexpr bool has_value() const {
        return m_val != nullptr;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return has_value();
    }

    [[nodiscard]] constexpr T &value() {
        if (!has_value()) {
            throw bad_optional_ref_access{};
        }
        return *m_val;
    }

    [[nodiscard]] constexpr T const &value() const {
        if (!has_value()) {
            throw bad_optional_ref_access{};
        }
        return *m_val;
    }

    [[nodiscard]] constexpr T &operator*() {
        return value();
    }

    [[nodiscard]] constexpr T const &operator*() const {
        return value();
    }

    [[nodiscard]] constexpr T *operator->() {
        return m_val;
    }

    [[nodiscard]] constexpr T const *operator->() const {
        assert(has_value());
        return m_val;
    }

    [[nodiscard]] constexpr T value_or(std::same_as<std::remove_const_t<T>> auto &&alt) const {
        if (has_value()) { return *m_val; }
        return std::forward<decltype(alt)>(alt);
    }

    constexpr void reset() noexcept {
        m_val = nullptr;
    }

    constexpr optional_ref &operator=(T &val) {
        m_val = std::addressof(val);
        return *this;
    }

    constexpr auto &operator=(T &&val) = delete;

    constexpr void swap(optional_ref &other) noexcept {
        std::swap(m_val, other.m_val);
    }

    [[nodiscard]] constexpr std::optional<T> as_owned() const requires std::copy_constructible<T>
    {
        if (!has_value()) { return std::nullopt; }
        return *m_val;
    }

    [[nodiscard]] constexpr std::optional<T> take_ownership()
        requires (!std::is_const_v<T> && std::movable<T>, std::move_constructible<T>)
    {
        if (!has_value()) { return std::nullopt; }
        auto *v = std::exchange(m_val, nullptr);
        return std::move(*v);
    }

private:
    T *m_val{nullptr};
};

template <typename T>
optional_ref(T &) -> optional_ref<T>;

template <typename T>
constexpr void swap(optional_ref<T> &lhs, optional_ref<T> &rhs) {
    lhs.swap(rhs);
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(optional_ref<T> const &lhs, optional_ref<U> const &rhs) {
    return ((!lhs && !rhs) || *lhs == *rhs);
}

template <typename T, std::three_way_comparable_with<T> U>
[[nodiscard]] constexpr std::compare_three_way_result_t<T, U> operator<=>(optional_ref<T> const &lhs, optional_ref<U> const &rhs) {
    using ordering_t = std::compare_three_way_result_t<T, U>;
    if (!lhs && !rhs) { return ordering_t::equivalent; }
    if (!lhs) { return ordering_t::less; }
    if (!rhs) { return ordering_t::greater; }
    return *lhs <=> *rhs;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(optional_ref<T> const &lhs, std::optional<U> const &rhs) {
    return ((!lhs && !rhs) || *lhs == *rhs);
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(std::optional<T> const &lhs, optional_ref<U> const &rhs) {
    return rhs == lhs;
}

template <typename T, std::three_way_comparable_with<T> U>
[[nodiscard]] constexpr std::compare_three_way_result_t<T, U> operator<=>(optional_ref<T> const &lhs, std::optional<U> const &rhs) {
    using ordering_t = std::compare_three_way_result_t<T, U>;
    if (!lhs && !rhs) { return ordering_t::equivalent; }
    if (!lhs) { return ordering_t::less; }
    if (!rhs) { return ordering_t::greater; }
    return *lhs <=> *rhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(optional_ref<T> const &lhs, std::nullopt_t) {
    return !lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(std::nullopt_t, optional_ref<T> const &rhs) {
    return rhs == std::nullopt;
}

template <typename T>
[[nodiscard]] constexpr std::strong_ordering operator<=>(optional_ref<T> const &lhs, std::nullopt_t) {
    if (!lhs) { return std::strong_ordering::equivalent; }
    return std::strong_ordering::greater;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(optional_ref<T> const &lhs, U const &rhs) {
    return lhs && *lhs == rhs;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(T const &lhs, optional_ref<U> const &rhs) {
    return rhs == lhs;
}

template <typename T, typename U>
[[nodiscard]] constexpr std::compare_three_way_result_t<T, U> operator<=>(optional_ref<T> const &lhs, U const &rhs) {
    using ordering_t = std::compare_three_way_result_t<T, U>;
    if (!lhs) { return ordering_t::less; }
    return *lhs <=> rhs;
}

}  // namespace cpputils

#endif
