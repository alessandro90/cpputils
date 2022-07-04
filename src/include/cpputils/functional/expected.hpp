#ifndef CPPUTILS_EXPECTED_HPP
#define CPPUTILS_EXPECTED_HPP

#include "../traits/is_specialization_of.hpp"
#include <concepts>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>


// TODO:
// - conversion between expected with different policies

// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils {

struct bad_expected_access : public std::exception {};
struct bad_expected_value_access : bad_expected_access {};
struct bad_expected_error_access : bad_expected_access {};

struct bad_expected_access_throw_policy {
    [[noreturn]] static void bad_value_access() {
        throw bad_expected_value_access{};
    }
    [[noreturn]] static void bad_error_access() {
        throw bad_expected_error_access{};
    }
};

template <typename Policy>
concept expected_bad_access_policy = requires {
    Policy::bad_value_access();
    Policy::bad_error_access();
};

struct flag_value_t {};
struct flag_error_t {};

inline constexpr auto flag_value = flag_value_t{};
inline constexpr auto flag_error = flag_error_t{};

namespace detail {

    template <template <typename> typename Condition, typename A, typename B>
    inline constexpr auto both_types_v = Condition<A>::value &&Condition<B>::value;

    template <template <typename> typename Condition, typename A, typename B>
    inline constexpr auto one_of_v = Condition<A>::value || Condition<B>::value;

    template <typename T, typename Q>
    concept same_as_no_ref = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Q>>;

    template <typename Arg, typename Type, bool IsPointer = std::is_pointer_v<Type>>
    struct is_valid_arg_for_costructor
        : std::conditional_t<std::is_same_v<std::remove_cvref_t<Type>,
                                            std::remove_cvref_t<Arg>>,
                             std::conditional_t<std::is_rvalue_reference_v<Arg>,
                                                std::is_move_constructible<std::remove_cvref_t<Arg>>,
                                                std::is_copy_constructible<std::remove_cvref_t<Arg>>>,
                             std::false_type> {};

    template <typename Arg, typename Type>
    struct is_valid_arg_for_costructor<Arg, Type, true>
        : std::conditional_t<std::is_constructible_v<Type, Arg>,
                             std::true_type,
                             std::conditional_t<!std::is_lvalue_reference_v<Arg>,
                                                std::false_type,
                                                std::conditional_t<!std::is_const_v<std::remove_pointer_t<Type>> && std::is_const_v<std::remove_cvref_t<Arg>>,
                                                                   std::false_type,
                                                                   std::true_type>>> {
    };

    template <typename Arg, typename Type, bool IsPointer = std::is_pointer_v<Type>>
    inline constexpr auto is_valid_arg_for_costructor_v = is_valid_arg_for_costructor<Arg, Type, IsPointer>::value;

    template <typename Arg, typename Type, bool IsPointer = std::is_pointer_v<Type>>
    concept expected_arg_constructible = is_valid_arg_for_costructor_v<Arg &&, Type, IsPointer>;

    template <typename T>
    struct is_noexcept_assign
        : std::conditional_t<std::is_lvalue_reference_v<T>,
                             std::is_nothrow_copy_assignable<std::remove_cvref_t<T>>,
                             std::is_nothrow_move_assignable<std::remove_cvref_t<T>>> {};

    template <typename T>
    inline constexpr auto is_noexcept_assign_v = is_noexcept_assign<T>::value;

    template <typename T>
    struct is_noexcept_construct
        : std::conditional_t<std::is_rvalue_reference_v<T>,
                             std::is_nothrow_move_constructible<std::remove_cvref_t<T>>,
                             std::is_nothrow_copy_constructible<std::remove_cvref_t<T>>> {};

    template <typename T>
    inline constexpr auto is_noexcept_construct_v = is_noexcept_construct<T>::value;


    template <template <typename> typename TrivialCtor, template <typename> typename CtorType, typename T>
    struct has_non_default_ctor
        : std::conditional_t<!TrivialCtor<T>::value && CtorType<T>::value,
                             std::true_type,
                             std::false_type> {};

    template <template <typename> typename TrivialCtor, template <typename> typename CtorType, typename T>
    inline constexpr auto has_non_default_ctor_v = has_non_default_ctor<TrivialCtor, CtorType, T>::value;

    template <typename T1, typename T2>
    inline constexpr auto has_non_default_copy_ctor_v = (has_non_default_ctor_v<std::is_trivially_copy_constructible, std::is_copy_assignable, T1> && std::is_copy_assignable_v<T2>)
                                                        || (has_non_default_ctor_v<std::is_trivially_copy_constructible, std::is_copy_assignable, T2> && std::is_copy_assignable_v<T1>);


    template <typename T1, typename T2>
    inline constexpr auto has_non_default_move_ctor_v = (has_non_default_ctor_v<std::is_trivially_move_constructible, std::is_move_assignable, T1> && std::is_move_assignable_v<T2>)
                                                        || (has_non_default_ctor_v<std::is_trivially_move_constructible, std::is_move_assignable, T2> && std::is_move_assignable_v<T1>);

    template <template <typename> typename TrivialAssign, template <typename> typename AssignType, template <typename> typename CtorType, typename T>
    struct has_non_default_assign
        : std::conditional_t<(!TrivialAssign<T>::value || !std::is_trivially_destructible_v<T>)&&AssignType<T>::value && CtorType<T>::value,
                             std::true_type,
                             std::false_type> {};

    template <template <typename> typename TrivialAssign, template <typename> typename AssignType, template <typename> typename CtorType, typename T>
    inline constexpr auto has_non_default_assign_v = has_non_default_assign<TrivialAssign, AssignType, CtorType, T>::value;

    template <typename T1, typename T2>
    inline constexpr auto has_non_default_copy_assign_v = (has_non_default_assign_v<std::is_trivially_copy_assignable, std::is_copy_assignable, std::is_copy_constructible, T1> && std::is_copy_assignable_v<T2> && std::is_copy_constructible_v<T2>)
                                                          || (has_non_default_assign_v<std::is_trivially_copy_assignable, std::is_copy_assignable, std::is_copy_constructible, T2> && std::is_copy_assignable_v<T1> && std::is_copy_constructible_v<T1>);

    template <typename T1, typename T2>
    inline constexpr auto has_non_default_move_assign_v = (has_non_default_assign_v<std::is_trivially_move_assignable, std::is_move_assignable, std::is_move_constructible, T1> && std::is_move_assignable_v<T2> && std::is_move_constructible_v<T2>)
                                                          || (has_non_default_assign_v<std::is_trivially_move_assignable, std::is_move_assignable, std::is_move_constructible, T2> && std::is_move_assignable_v<T1> && std::is_move_constructible_v<T1>);

    struct expected_resource_helper {
        template <typename Expected>
        static constexpr void copy(Expected &lhs, auto &&rhs) noexcept(both_types_v<std::is_nothrow_destructible, typename Expected::value_t, typename Expected::error_t>
                                                                           &&both_types_v<is_noexcept_assign, typename Expected::value_t, typename Expected::error_t>) {
            using value_t = Expected::value_t;  // NOLINT
            using error_t = Expected::error_t;  // NOLINT
            if (lhs) {
                if (rhs) {
                    lhs.m_val = FWD(rhs).m_val;
                } else {
                    if constexpr (!std::is_trivially_destructible_v<value_t>) {
                        destroy(flag_value, lhs);
                    }
                    construct(flag_error, lhs, FWD(rhs).m_err);
                }
            } else {
                if (!rhs) {
                    lhs.m_err = FWD(rhs).m_err;
                } else {
                    if constexpr (!std::is_trivially_destructible_v<error_t>) {
                        destroy(flag_error, lhs);
                    }
                    construct(flag_value, lhs, FWD(rhs).m_val);
                }
            }
        }

        template <typename Expected>
        static constexpr void initialize(Expected &lhs, auto &&rhs) noexcept(both_types_v<std::is_nothrow_move_assignable,
                                                                                          typename Expected::value_t,
                                                                                          typename Expected::error_t>) {
            if (rhs) {
                lhs.m_val = FWD(rhs).m_val;
                lhs.has_val = true;
            } else {
                lhs.m_err = FWD(rhs).m_err;
                lhs.has_val = false;
            }
        }

        template <typename Expected>
        // clang-format off
        static constexpr void destroy(Expected &&e) noexcept(both_types_v<std::is_nothrow_destructible,
                                                                          typename std::remove_cvref_t<Expected>::value_t,
                                                                          typename std::remove_cvref_t<Expected>::error_t>) {
            // clang-format on
            using type = std::remove_cvref_t<Expected>;
            using value_t = typename type::value_t;  // NOLINT
            using error_t = typename type::error_t;  // NOLINT
            if (e) {
                if constexpr (!std::is_trivially_destructible_v<value_t>) {
                    destroy(flag_value, FWD(e));
                }
            } else {
                if constexpr (!std::is_trivially_destructible_v<error_t>) {
                    destroy(flag_error, FWD(e));
                }
            }
        }

        static constexpr void construct(flag_value_t, auto &&e, auto &&v) noexcept(is_noexcept_construct_v<decltype(v)>) {
            std::construct_at(std::addressof(e.m_val), FWD(v));
            e.has_val = true;
        }
        static constexpr void construct(flag_error_t, auto &&e, auto &&v) noexcept(is_noexcept_construct_v<decltype(v)>) {
            std::construct_at(std::addressof(e.m_err), FWD(v));
            e.has_val = false;
        }

    private:
        static constexpr void destroy(flag_value_t, auto &&e) noexcept(std::is_nothrow_destructible_v<decltype(e.m_val)>) {
            std::destroy_at(std::addressof(e.m_val));
        }
        static constexpr void destroy(flag_error_t, auto &&e) noexcept(std::is_nothrow_destructible_v<decltype(e.m_err)>) {
            std::destroy_at(std::addressof(e.m_err));
        }
    };

    template <typename Value_t, typename Error_t, expected_bad_access_policy ErrorPolicy_t>
    struct expected_data {
        static_assert(!std::is_same_v<std::remove_cvref_t<Value_t>, void>);
        static_assert(!std::is_same_v<std::remove_cvref_t<Error_t>, void>);
        static_assert(!std::is_same_v<std::remove_cvref_t<ErrorPolicy_t>, void>);

        template <typename A, typename B, expected_bad_access_policy C>
        friend struct expected_data;

        friend expected_resource_helper;

        using value_t = std::conditional_t<std::is_reference_v<Value_t>,
                                           std::add_pointer_t<std::remove_reference_t<Value_t>>,
                                           Value_t>;

        using error_t = std::conditional_t<std::is_reference_v<Error_t>,
                                           std::add_pointer_t<std::remove_reference_t<Error_t>>,
                                           Error_t>;

        constexpr expected_data() = delete;

        template <expected_arg_constructible<value_t> Arg>
        constexpr expected_data(flag_value_t, Arg &&v) requires(!is_specialization_v<Arg, expected_data>)
            : m_val{get_arg(FWD(v))}
            , has_val{true} {}

        template <expected_arg_constructible<error_t> Arg>
        constexpr expected_data(flag_error_t, Arg &&v) requires(!is_specialization_v<Arg, expected_data>)
            : m_err{get_arg(FWD(v))} {}

        constexpr expected_data(flag_value_t, std::in_place_t, auto &&...args) requires std::negation_v<std::is_pointer<value_t>> && std::constructible_from<value_t, decltype(args)...>
            : m_val{FWD(args)...},
              has_val{true} {}

        constexpr expected_data(flag_error_t, std::in_place_t, auto &&...args) requires std::negation_v<std::is_pointer<error_t>> && std::constructible_from<error_t, decltype(args)...>
            : m_err{FWD(args)...} {}

        template <expected_bad_access_policy C>
        explicit constexpr expected_data(expected_data<Value_t, Error_t, C> const &other) {
            expected_resource_helper::initialize(*this, other);
        }

        template <expected_bad_access_policy C>
        explicit constexpr expected_data(expected_data<Value_t, Error_t, C> &&other) {
            expected_resource_helper::initialize(*this, std::move(other));
        }

        constexpr expected_data(expected_data const &) = delete;

        constexpr expected_data(expected_data const &) requires both_types_v<std::is_trivially_copy_constructible, value_t, error_t>
        = default;

        constexpr expected_data(expected_data const &other) requires has_non_default_copy_ctor_v<value_t, error_t> {
            expected_resource_helper::initialize(*this, other);
        }

        constexpr expected_data(expected_data &&) noexcept = delete;

        constexpr expected_data(expected_data &&) noexcept requires both_types_v<std::is_trivially_move_constructible, value_t, error_t>
        = default;

        constexpr expected_data(expected_data &&other) noexcept(
            noexcept(
                expected_resource_helper::initialize(std::declval<expected_data &>(),
                                                     std::declval<std::remove_cvref_t<decltype(other)>>()))) requires has_non_default_move_ctor_v<value_t, error_t> {
            expected_resource_helper::initialize(*this, std::move(other));
        }

        constexpr expected_data &operator=(expected_data const &) = delete;

        constexpr expected_data &operator=(expected_data const &) requires both_types_v<std::is_trivially_copy_assignable, value_t, error_t>  //
            && both_types_v<std::is_trivially_destructible, value_t, error_t>  //
            && both_types_v<std::is_trivially_copy_constructible, value_t, error_t>
        = default;

        constexpr expected_data &operator=(expected_data const &rhs) requires has_non_default_copy_assign_v<value_t, error_t> {  // NOLINT(cert-oop54-cpp)
            expected_resource_helper::copy(*this, rhs);
            return *this;
        }

        constexpr expected_data &operator=(expected_data &&) noexcept = delete;

        constexpr expected_data &operator=(expected_data &&) noexcept requires both_types_v<std::is_trivially_move_assignable, value_t, error_t>  //
            && both_types_v<std::is_trivially_destructible, value_t, error_t>  //
            && both_types_v<std::is_trivially_move_constructible, value_t, error_t>
        = default;

        constexpr expected_data &operator=(expected_data &&rhs) noexcept(
            noexcept(expected_resource_helper::copy(std::declval<expected_data &>(),
                                                    std::declval<std::remove_cvref_t<decltype(rhs)>>))) requires has_non_default_move_assign_v<value_t, error_t> {
            expected_resource_helper::copy(*this, std::move(rhs));
            return *this;
        }

        template <expected_bad_access_policy C>
        constexpr expected_data &operator=(expected_data<Value_t, Error_t, C> const &rhs) {
            expected_resource_helper::copy(*this, rhs);
            return *this;
        }

        template <expected_bad_access_policy C>
        constexpr expected_data &operator=(expected_data<Value_t, Error_t, C> &&rhs) {
            expected_resource_helper::copy(*this, std::move(rhs));
            return *this;
        }

        constexpr ~expected_data() requires both_types_v<std::is_trivially_destructible, value_t, error_t>
        = default;

        // NOTE: noexcept should be reduntant
        constexpr ~expected_data() noexcept(noexcept(expected_resource_helper::destroy(std::declval<expected_data &>()))) {
            expected_resource_helper::destroy(*this);
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return has_val;
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return has_value();
        }

        [[nodiscard]] constexpr decltype(auto) value() & {
            return get_val(*this);
        }
        [[nodiscard]] constexpr decltype(auto) value() const & {
            return get_val(*this);
        }

        [[nodiscard]] constexpr decltype(auto) value() && {
            return get_val(std::move(*this));
        }
        [[nodiscard]] constexpr decltype(auto) value() const && {
            return get_val(std::move(*this));
        }

        [[nodiscard]] constexpr decltype(auto) error() & {
            return get_err(*this);
        }
        [[nodiscard]] constexpr decltype(auto) error() const & {
            return get_err(*this);
        }

        [[nodiscard]] constexpr decltype(auto) error() && {
            return get_err(std::move(*this));
        }
        [[nodiscard]] constexpr decltype(auto) error() const && {
            return get_err(std::move(*this));
        }

        //
        [[nodiscard]] constexpr decltype(auto) value_unchecked() & {
            return get_val_unchecked(*this);
        }
        [[nodiscard]] constexpr decltype(auto) value_unchecked() const & {
            return get_val_unchecked(*this);
        }

        [[nodiscard]] constexpr decltype(auto) value_unchecked() && {
            return get_val_unchecked(std::move(*this));
        }
        [[nodiscard]] constexpr decltype(auto) value_unchecked() const && {
            return get_val_unchecked(std::move(*this));
        }

        [[nodiscard]] constexpr decltype(auto) error_unchecked() & {
            return get_err_unchecked(*this);
        }
        [[nodiscard]] constexpr decltype(auto) error_unchecked() const & {
            return get_err_unchecked(*this);
        }

        [[nodiscard]] constexpr decltype(auto) error_unchecked() && {
            return get_err_unchecked(std::move(*this));
        }
        [[nodiscard]] constexpr decltype(auto) error_unchecked() const && {
            return get_err_unchecked(std::move(*this));
        }

        template <typename Policy>
        constexpr void swap(expected_data<Value_t, Error_t, Policy> &other) noexcept(
            // clang-format off
            std::is_nothrow_swappable_v<value_t> &&
            std::is_nothrow_swappable_v<error_t> &&
            noexcept(expected_resource_helper::copy(*this, std::move(other))) &&
            noexcept(expected_resource_helper::destroy(other)) &&
            noexcept(expected_resource_helper::construct(flag_value, other, std::declval<value_t>())) &&
            noexcept(expected_resource_helper::copy(*this, std::move(other))) &&
            noexcept(expected_resource_helper::destroy(other)) &&
            noexcept(expected_resource_helper::construct(flag_error, other, std::declval<error_t>()))) {
            // clang-format on
            using std::swap;
            if (*this) {
                if (other) {
                    swap(value_unchecked(), other.value_unchecked());
                } else {
                    auto v = value_unchecked();
                    expected_resource_helper::copy(*this, std::move(other));
                    expected_resource_helper::destroy(other);
                    expected_resource_helper::construct(flag_value, other, std::move(v));
                }
            } else {
                if (!other) {
                    swap(error_unchecked(), other.error_unchecked());
                } else {
                    auto e = error_unchecked();
                    expected_resource_helper::copy(*this, std::move(other));
                    expected_resource_helper::destroy(other);
                    expected_resource_helper::construct(flag_error, other, std::move(e));
                }
            }
        }

    private:
        union {
            value_t m_val;
            error_t m_err;
        };
        bool has_val{};

        static constexpr decltype(auto) get_arg(auto &&v) {
            if constexpr (std::is_reference_v<Value_t>) {
                return std::addressof(v);
            } else {
                return FWD(v);
            }
        }

        static constexpr decltype(auto) get_val_unchecked(auto &&self) {
            if constexpr (std::is_reference_v<Value_t>) {
                return *self.m_val;
            } else {
                return (FWD(self).m_val);
            }
        }

        static constexpr decltype(auto) get_err_unchecked(auto &&self) {
            if constexpr (std::is_reference_v<Error_t>) {
                return *self.m_err;
            } else {
                return (FWD(self).m_err);
            }
        }

        static constexpr decltype(auto) get_val(auto &&self) {
            if (!self) { ErrorPolicy_t::bad_value_access(); }
            return get_val_unchecked(FWD(self));
        }

        static constexpr decltype(auto) get_err(auto &&self) {
            if (self) { ErrorPolicy_t::bad_error_access(); }
            return get_err_unchecked(FWD(self));
        }
    };


}  // namespace detail

template <typename Value_t, typename Error_t, expected_bad_access_policy ErrorPolicy_t = bad_expected_access_throw_policy>
class expected
    : private detail::expected_data<Value_t, Error_t, ErrorPolicy_t> {
    using base_expected = detail::expected_data<Value_t, Error_t, ErrorPolicy_t>;
    using base_expected::value_unchecked;
    using base_expected::error_unchecked;

public:
    using base_expected::value_t;
    using base_expected::error_t;

    using base_expected::base_expected;  // Bring constructors in scope

    using base_expected::value;
    using base_expected::error;

    using base_expected::has_value;
    using base_expected::operator bool;
    using base_expected::swap;

    template <typename A, typename B, expected_bad_access_policy C>
    friend class expected;

    template <typename ErrorPolicyB_t>
    [[nodiscard]] constexpr bool operator==(expected<Value_t, Error_t, ErrorPolicyB_t> const &rhs) const {
        if (*this && rhs) {
            return value_unchecked() == rhs.value_unchecked();
        } else if (!*this && !rhs) {
            return error_unchecked() == rhs.error_unchecked();
        } else {
            return false;
        }
    }

    template <expected_bad_access_policy C>
    explicit constexpr expected(expected<Value_t, Error_t, C> const &other)
        : base_expected{other} {}

    template <expected_bad_access_policy C>
    explicit constexpr expected(expected<Value_t, Error_t, C> &other)
        : base_expected{std::move(other)} {}

    template <expected_bad_access_policy C>
    constexpr expected &operator=(expected<Value_t, Error_t, C> const &other) {
        base_expected::operator=(other);
        return *this;
    }

    template <expected_bad_access_policy C>
    constexpr expected &operator=(expected<Value_t, Error_t, C> &&other) {
        base_expected::operator=(std::move(other));
        return *this;
    }

    template <detail::expected_arg_constructible<typename base_expected::value_t> Arg>
    [[nodiscard]] static constexpr auto val(Arg &&arg) {
        return expected{flag_value, FWD(arg)};
    }
    template <detail::expected_arg_constructible<typename base_expected::error_t> Arg>
    [[nodiscard]] static constexpr auto err(Arg &&arg) {
        return expected{flag_error, FWD(arg)};
    }

    [[nodiscard]] constexpr decltype(auto) transform(auto &&f) & {
        return transform_impl(*this, FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform(auto &&f) const & {
        return transform_impl(*this, FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform(auto &&f) && {
        return transform_impl(std::move(*this), FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform(auto &&f) const && {
        return transform_impl(std::move(*this), FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform_error(auto &&f) & {
        return transform__error_impl(*this, FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform_error(auto &&f) const & {
        return transform__error_impl(*this, FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform_error(auto &&f) && {
        return transform__error_impl(std::move(*this), FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) transform_error(auto &&f) const && {
        return transform__error_impl(std::move(*this), FWD(f));
    }

    [[nodiscard]] constexpr decltype(auto) value_or(auto &&alternative) & {
        return value_or_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or(auto &&alternative) const & {
        return value_or_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or(auto &&alternative) && {
        return value_or_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or(auto &&alternative) const && {
        return value_or_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or_else(auto &&alternative) & {
        return value_or_else_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or_else(auto &&alternative) const & {
        return value_or_else_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or_else(auto &&alternative) && {
        return value_or_else_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) value_or_else(auto &&alternative) const && {
        return value_or_else_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or(auto &&alternative) & {
        return error_or_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or(auto &&alternative) const & {
        return error_or_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or(auto &&alternative) && {
        return error_or_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or(auto &&alternative) const && {
        return error_or_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or_else(auto &&alternative) & {
        return error_or_else_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or_else(auto &&alternative) const & {
        return error_or_else_impl(*this, FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or_else(auto &&alternative) && {
        return error_or_else_impl(std::move(*this), FWD(alternative));
    }

    [[nodiscard]] constexpr decltype(auto) error_or_else(auto &&alternative) const && {
        return error_or_else_impl(std::move(*this), FWD(alternative));
    }

    constexpr decltype(auto) if_value(auto &&cb) & {
        return if_value_impl(*this, FWD(cb));
    }
    constexpr decltype(auto) if_value(auto &&cb) const & {
        return if_value_impl(*this, FWD(cb));
    }
    constexpr decltype(auto) if_value(auto &&cb) && {
        return if_value_impl(std::move(*this), FWD(cb));
    }
    constexpr decltype(auto) if_value(auto &&cb) const && {
        return if_value_impl(std::move(*this), FWD(cb));
    }

    constexpr decltype(auto) or_else(auto &&cb) & {
        return or_else_impl(*this, FWD(cb));
    }
    constexpr decltype(auto) or_else(auto &&cb) const & {
        return or_else_impl(*this, FWD(cb));
    }
    constexpr decltype(auto) or_else(auto &&cb) && {
        return or_else_impl(std::move(*this), FWD(cb));
    }
    constexpr decltype(auto) or_else(auto &&cb) const && {
        return or_else_impl(std::move(*this), FWD(cb));
    }

    [[nodiscard]] constexpr auto get_hash() const {
        if (*this) { return std::hash<std::remove_const_t<Value_t>>{}(value_unchecked()); }
        return std::hash<std::remove_const_t<Error_t>>{}(error_unchecked());
    }

    // clang-format off
private:
    static constexpr auto transform_impl(auto &&e, std::invocable<decltype(FWD(e).value_unchecked())> auto &&f)
    requires is_specialization_v<std::invoke_result_t<decltype(f), decltype(FWD(e).value_unchecked())>, expected> {
        // clang-format on
        using ret_t = std::invoke_result_t<decltype(f), decltype(FWD(e).value_unchecked())>;
        static_assert(std::is_same_v<typename base_expected::error_t, typename ret_t::error_t>);
        if (e) { return std::invoke(FWD(f), FWD(e).value_unchecked()); }
        return ret_t::err(FWD(e).error_unchecked());
    }

    static constexpr auto transform_impl(auto &&e, std::invocable<decltype(FWD(e).value_unchecked())> auto &&f) {
        using ret_t = std::invoke_result_t<decltype(f), decltype(FWD(e).value_unchecked())>;
        using expected_ret_t = expected<ret_t, typename base_expected::error_t>;
        if (e) { return expected_ret_t::val(std::invoke(FWD(f), FWD(e).value_unchecked())); }
        return expected_ret_t::err(FWD(e).error_unchecked());
    }

    static constexpr auto transform_error_impl(auto &&e, std::invocable<decltype(FWD(e).error_unchecked())> auto &&f)  //
        requires is_specialization_v<std::invoke_result_t<decltype(f), decltype(FWD(e).error_unchecked())>, expected> {
        using ret_t = std::invoke_result_t<decltype(f), decltype(FWD(e).error_unchecked())>;
        static_assert(std::is_same_v<typename base_expected::value_t, typename ret_t::value_t>);
        if (!e) { return std::invoke(FWD(f), FWD(e).error_unchecked()); }
        return ret_t::val(FWD(e).value_unchecked());
    }

    static constexpr auto transform_error_impl(auto &&e, std::invocable<decltype(FWD(e).error_unchecked())> auto &&f) {
        using ret_t = std::invoke_result_t<decltype(f), decltype(FWD(e).error_unchecked())>;
        using expected_ret_t = expected<typename base_expected::value_t, ret_t>;
        if (!e) { return expected_ret_t::err(std::invoke(FWD(f), FWD(e).error_unchecked())); }
        return expected_ret_t::val(FWD(e).value_unchecked());
    }

    static constexpr auto value_or_impl(auto &&e, detail::same_as_no_ref<decltype(FWD(e).value_unchecked())> auto &&alternative)
        -> std::add_lvalue_reference_t<std::add_const_t<std::remove_cvref_t<decltype(FWD(e).value_unchecked())>>> {
        if (e) { return FWD(e).value_unchecked(); }
        return FWD(alternative);
    }

    static constexpr auto value_or_else_impl(auto &&e, std::invocable<decltype(FWD(e).error_unchecked())> auto &&alternative)  //
        requires std::is_convertible_v<decltype(FWD(e).value_unchecked()), decltype(std::invoke(FWD(alternative), FWD(e).error_unchecked()))> {
        if (e) { return FWD(e).value_unchecked(); }
        return std::invoke(FWD(alternative), FWD(e).error_unchecked());
    }

    static constexpr auto error_or_impl(auto &&e, detail::same_as_no_ref<decltype(FWD(e).error_unchecked())> auto &&alternative)
        -> std::add_lvalue_reference_t<std::add_const_t<std::remove_cvref_t<decltype(FWD(e).error_unchecked())>>> {
        if (!e) { return FWD(e).error_unchecked(); }
        return FWD(alternative);
    }

    static constexpr auto error_or_else_impl(auto &&e, std::invocable<decltype(FWD(e).value_unchecked())> auto &&alternative)  //
        requires std::is_convertible_v<decltype(FWD(e).error_unchecked()), decltype(std::invoke(FWD(alternative), FWD(e).value_unchecked()))> {
        if (!e) { return FWD(e).error_unchecked(); }
        return std::invoke(FWD(alternative), FWD(e).value_unchecked());
    }

    static constexpr decltype(auto) if_value_impl(auto &&e, std::invocable<decltype(FWD(e).value_unchecked())> auto &&cb) {
        if (e) { std::invoke(FWD(e).value_unchecked(), FWD(cb)); }
        return FWD(e);
    }

    static constexpr decltype(auto) or_else_impl(auto &&e, std::invocable<decltype(FWD(e).error_unchecked())> auto &&cb) {
        if (!e) { std::invoke(FWD(e).error_unchecked(), FWD(cb)); }
        return FWD(e);
    }
};

template <typename Value_t, typename Error_t, typename Policy_A, typename Policy_B>
constexpr void swap(::cpputils::expected<Value_t, Error_t, Policy_A> &lhs,
                    ::cpputils::expected<Value_t, Error_t, Policy_B> &rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

}  // namespace cpputils

namespace std {  // NOLINT(cert-dcl58-cpp)

template <typename Value_t, typename Error_t, typename Policy_t>
struct hash<::cpputils::expected<Value_t, Error_t, Policy_t>> {
    constexpr auto operator()(::cpputils::expected<Value_t, Error_t, Policy_t> const &e) const {
        return e.get_hash();
    }
};

}  // namespace std
#undef FWD
#endif
