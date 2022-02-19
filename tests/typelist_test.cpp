#include "cpputils/traits/typelist.hpp"
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <tuple>

using namespace cpputils::tl;

namespace {
template <typename A, typename B>
consteval void check_same() { static_assert(std::is_same_v<A, B>); }

struct A;
struct B;
struct C;
struct D;
struct E;
struct F;
struct G;
struct H;

enum class Enum : std::uint8_t {};

template <typename T, typename E>
struct take_bigger {
    using type = std::conditional_t<sizeof(T) >= sizeof(E), T, E>;
};
}  // namespace

TEST_CASE("Typelist static assertions") {
    check_same<head<typelist<A, B, C>>::list, typelist<A>>();
    check_same<head<typelist<>>::list, typelist<>>();
    check_same<head<>::list<typelist<>>, typelist<>>();
    check_same<head<>::list<typelist<A, B, C>>, typelist<A>>();

    check_same<tail<typelist<A, B, C>>::list, typelist<B, C>>();
    check_same<tail<typelist<C>>::list, typelist<>>();
    check_same<tail<>::list<typelist<>>, typelist<>>();
    check_same<tail<>::list<typelist<A, B, C>>, typelist<B, C>>();

    check_same<append<A, typelist<>>::list, typelist<A>>();
    check_same<append<A, typelist<C>>::list, typelist<C, A>>();
    using append_closure = append<A>;
    check_same<append_closure::list<typelist<>>, typelist<A>>();
    check_same<append_closure::list<typelist<C>>, typelist<C, A>>();

    check_same<prepend<A, typelist<>>::list, typelist<A>>();
    check_same<prepend<A, typelist<B, C>>::list, typelist<A, B, C>>();
    using prepend_closure = prepend<A>;
    check_same<prepend_closure::list<typelist<>>, typelist<A>>();
    check_same<prepend_closure::list<typelist<C>>, typelist<A, C>>();

    check_same<join<typelist<A, B, C>, typelist<D>>::list, typelist<A, B, C, D>>();
    check_same<join<typelist<A, B, C>, typelist<>>::list, typelist<A, B, C>>();
    check_same<join<typelist<>, typelist<D>>::list, typelist<D>>();
    check_same<join<typelist<>, typelist<>>::list, typelist<>>();
    check_same<join<typelist<A, B, C>, typelist<D>, typelist<E, F, G>>::list,
               typelist<A, B, C, D, E, F, G>>();

    check_same<take<2, typelist<A, B, E>>::list, typelist<A, B>>();
    check_same<take<4, typelist<A, B, E>>::list, typelist<A, B, E>>();
    check_same<take<1, typelist<>>::list, typelist<>>();
    check_same<take<2>::list<typelist<A, B, C, D>>, typelist<A, B>>();

    check_same<as<std::tuple, typelist<A, B, C>>::type, std::tuple<A, B, C>>();

    static_assert(len<typelist<>>::value == 0);
    static_assert(len<typelist<A>>::value == 1);
    static_assert(len<typelist<A, D>>::value == 2);

    static_assert(empty<typelist<>>::value);
    static_assert(!empty<typelist<A>>::value);
    static_assert(!empty<typelist<A, G>>::value);

    check_same<last<typelist<>>::list, typelist<>>();
    check_same<last<typelist<A>>::list, typelist<A>>();
    check_same<last<typelist<A, C>>::list, typelist<C>>();
    check_same<last<>::list<typelist<A, B, C>>, typelist<C>>();

    check_same<pop_front<typelist<>>::list, typelist<>>();
    check_same<pop_front<typelist<>>::front, typelist<>>();
    check_same<pop_front<typelist<A>>::list, typelist<>>();
    check_same<pop_front<typelist<A>>::front, typelist<A>>();
    check_same<pop_front<typelist<A, C>>::list, typelist<A>>();
    check_same<pop_front<typelist<A, C>>::front, typelist<C>>();
    check_same<pop_front<typelist<A, C, B>>::list, typelist<A, C>>();
    check_same<pop_front<typelist<A, C, B>>::front, typelist<B>>();

    check_same<pop_back<typelist<>>::list, typelist<>>();
    check_same<pop_back<typelist<>>::back, typelist<>>();
    check_same<pop_back<typelist<A>>::list, typelist<>>();
    check_same<pop_back<typelist<A>>::back, typelist<A>>();
    check_same<pop_back<typelist<A, C>>::list, typelist<C>>();
    check_same<pop_back<typelist<A, C>>::back, typelist<A>>();
    check_same<pop_back<typelist<A, C, B>>::list, typelist<C, B>>();
    check_same<pop_back<typelist<A, C, B>>::back, typelist<A>>();

    check_same<reverse<typelist<>>::list, typelist<>>();
    check_same<reverse<typelist<A>>::list, typelist<A>>();
    check_same<reverse<typelist<A, C>>::list, typelist<C, A>>();
    check_same<reverse<typelist<A, C, B>>::list, typelist<B, C, A>>();
    check_same<reverse<>::list<typelist<A, B, C>>, typelist<C, B, A>>();

    check_same<filter<std::is_integral, typelist<>>::list, typelist<>>();
    check_same<filter<std::is_integral,
                      typelist<B, std::int32_t, D, std::int8_t, C, H>>::list,
               typelist<std::int32_t, std::int8_t>>();
    check_same<filter<std::is_integral>::list<
                   typelist<B, std::int32_t, D, std::int8_t, C, H>>,
               typelist<std::int32_t, std::int8_t>>();

    check_same<repeat<0, A>::list, typelist<>>();
    check_same<repeat<2, A>::list, typelist<A, A>>();

    check_same<take_while<std::is_floating_point,
                          typelist<>>::list,
               typelist<>>();
    check_same<take_while<std::is_floating_point,
                          typelist<float>>::list,
               typelist<float>>();
    check_same<take_while<std::is_floating_point,
                          typelist<float, double, float, C, B>>::list,
               typelist<float, double, float>>();
    check_same<take_while<std::is_floating_point>::list<
                   typelist<float, double, float, C, B>>,
               typelist<float, double, float>>();

    check_same<flat<typelist<typelist<>>>::list, typelist<>>();
    check_same<flat<typelist<typelist<>, typelist<>>>::list, typelist<>>();
    check_same<flat<typelist<typelist<A>, typelist<>>>::list, typelist<A>>();
    check_same<flat<typelist<typelist<A>, typelist<B>>>::list, typelist<A, B>>();
    check_same<flat<typelist<typelist<A>, typelist<B, C>>>::list, typelist<A, B, C>>();
    check_same<flat<typelist<typelist<B, C>, typelist<A>>>::list, typelist<B, C, A>>();
    check_same<flat<typelist<typelist<B, C, typelist<A>>, typelist<A>>>::list, typelist<B, C, A, A>>();
    check_same<flat<typelist<typelist<A>, typelist<B, C, typelist<A>>>>::list, typelist<A, B, C, A>>();
    check_same<flat<>::list<typelist<typelist<A>, typelist<B, C, typelist<A>>>>, typelist<A, B, C, A>>();

    check_same<zip<typelist<>, typelist<>>::list, typelist<>>();
    check_same<zip<typelist<A, B>, typelist<C, D>>::list, typelist<typelist<A, C>, typelist<B, D>>>();
    check_same<zip<typelist<A, B>, typelist<C, D>, typelist<E, F>>::list,
               typelist<typelist<A, C, E>, typelist<B, D, F>>>();

    check_same<unique<typelist<A, B, A, C, D, A, B, E>>::list, typelist<A, B, C, D, E>>();
    check_same<unique<>::list<typelist<A, B, A, C, D, A, B, E>>, typelist<A, B, C, D, E>>();
    check_same<unique<typelist<>>::list, typelist<>>();

    check_same<map<std::make_unsigned,
                   typelist<std::int32_t,
                            std::int8_t,
                            std::uint32_t,
                            std::int16_t>>::list,
               typelist<std::uint32_t,
                        std::uint8_t,
                        std::uint32_t,
                        std::uint16_t>>();
    check_same<map<std::make_unsigned, typelist<>>::list, typelist<>>();
    check_same<map<std::make_unsigned>::list<
                   typelist<std::int32_t,
                            std::int8_t,
                            std::uint32_t,
                            std::int16_t>>,
               typelist<std::uint32_t,
                        std::uint8_t,
                        std::uint32_t,
                        std::uint16_t>>();

    static_assert(count<A, typelist<A, B, C, A, E>>::value == 2);
    static_assert(count<A, typelist<B, B, C, F, E>>::value == 0);
    static_assert(count<A, typelist<>>::value == 0);

    static_assert(count_if<std::is_unsigned, typelist<>>::value == 0);
    static_assert(count_if<std::is_unsigned,
                           typelist<std::int32_t,
                                    std::uint32_t,
                                    std::int16_t,
                                    std::uint16_t,
                                    std::uint8_t>>::value
                  == 3);

    check_same<drop<2, typelist<A, B, C, D>>::list, typelist<C, D>>();
    check_same<drop<2>::list<typelist<A, B, C, D>>, typelist<C, D>>();
    check_same<drop<2, typelist<>>::list, typelist<>>();
    check_same<drop<2, typelist<A>>::list, typelist<>>();

    check_same<drop_while<std::is_integral,
                          typelist<std::int32_t,
                                   std::int8_t,
                                   std::int16_t,
                                   float,
                                   std::int32_t>>::list,
               typelist<float, std::int32_t>>();
    check_same<drop_while<std::is_integral>::list<
                   typelist<std::int32_t,
                            std::int8_t,
                            std::int16_t,
                            float,
                            std::int32_t>>,
               typelist<float, std::int32_t>>();
    check_same<drop_while<std::is_integral, typelist<>>::list, typelist<>>();

    check_same<enumerate<typelist<A, B, C, D>>::list,
               typelist<enumeration<0, A>,
                        enumeration<1, B>,
                        enumeration<2, C>,
                        enumeration<3, D>>>();
    check_same<enumerate<>::list<typelist<A, B, C, D>>,
               typelist<enumeration<0, A>,
                        enumeration<1, B>,
                        enumeration<2, C>,
                        enumeration<3, D>>>();
    check_same<enumerate<typelist<A>>::list, typelist<enumeration<0, A>>>();


    check_same<zip_with<take_bigger, typelist<>, typelist<>>::list, typelist<>>();
    check_same<zip_with<take_bigger,
                        typelist<std::int32_t,
                                 std::int8_t,
                                 std::int16_t,
                                 double>,
                        typelist<std::int16_t,
                                 std::int32_t,
                                 std::int64_t,
                                 std::int8_t>>::list,
               typelist<std::int32_t,
                        std::int32_t,
                        std::int64_t,
                        double>>();

    check_same<fold<take_bigger,
                    typelist<std::int8_t,
                             std::int32_t,
                             std::int16_t,
                             std::int64_t,
                             std::uint8_t>>::list,
               typelist<std::int64_t>>();
    check_same<fold<take_bigger>::list<
                   typelist<std::int8_t,
                            std::int32_t,
                            std::int16_t,
                            std::int64_t,
                            std::uint8_t>>,
               typelist<std::int64_t>>();

    static_assert(compose_predicate<std::is_integral, std::is_signed>::composed<std::int32_t>::value);
    static_assert(!compose_predicate<std::is_integral, std::is_signed>::composed<std::uint32_t>::value);

    check_same<compose_transformation<std::underlying_type, std::make_signed>::composed<Enum>::type, std::int8_t>();

    using take_first_3_unique_signed_integrals = compose<unique<>,
                                                         filter<compose_predicate<std::is_integral,
                                                                                  std::is_signed>::composed>,
                                                         take<3>>;
    check_same<take_first_3_unique_signed_integrals::list<typelist<std::uint32_t,
                                                                   std::int32_t,
                                                                   float,
                                                                   std::int8_t,
                                                                   double,
                                                                   std::uint32_t,
                                                                   float,
                                                                   std::int8_t,
                                                                   std::int16_t,
                                                                   std::uint64_t,
                                                                   std::int8_t,
                                                                   std::int32_t,
                                                                   std::int16_t>>,
               typelist<std::int32_t,
                        std::int8_t,
                        std::int16_t>>();
}