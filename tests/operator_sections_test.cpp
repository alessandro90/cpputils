#include "catch2/catch_test_macros.hpp"
#define CPPULTILS_ENABLE_CALL_MACROS
#include "cpputils/functional/operator_sections.hpp"

using cpputils::_;  // NOLINT


namespace {
struct Obj {
    [[nodiscard]] int get() const { return v; }
    [[nodiscard]] Obj square() const {
        return Obj{v * v};
    }
    [[nodiscard]] int mult(int m) const {
        return v * m;
    }
    int v{42};
};
}  // namespace

// NOLINTNEXTLINE
TEST_CASE("sections test", "") {
    SECTION("operator +") {
        auto const sum = _ + _;
        REQUIRE(sum(1, 2) == 3);
    }
    SECTION("partial operator +") {
        {
            auto const add_10 = _ + 10;
            REQUIRE(add_10(1) == 11);
        }
        {
            auto const add_10 = 10 + _;
            REQUIRE(add_10(1) == 11);
        }
    }
    ///
    SECTION("partial operator -") {
        {
            auto const subtract_10 = _ - 10;
            REQUIRE(subtract_10(1) == -9);
        }
        {
            auto const subtract_10 = 10 - _;
            REQUIRE(subtract_10(10) == 0);
        }
    }
    SECTION("operator -") {
        auto const subtract = _ - _;
        REQUIRE(subtract(1, 2) == -1);
    }
    SECTION("Unary minus") {
        auto const negate = -_;
        REQUIRE(negate(1) == -1);
    }
    ///
    SECTION("operator *") {
        auto const mul = _ * _;
        REQUIRE(mul(3, 2) == 6);
    }
    SECTION("partial operator *") {
        {
            auto const mul_10 = _ * 10;
            REQUIRE(mul_10(2) == 20);
        }
        {
            auto const mul_10 = 10 * _;
            REQUIRE(mul_10(2) == 20);
        }
    }
    ///
    SECTION("operator /") {
        auto const div = _ / _;
        REQUIRE(div(6, 2) == 3);
    }
    SECTION("partial operator /") {
        {
            auto const div_10 = _ / 10;
            REQUIRE(div_10(20) == 2);
        }
        {
            auto const ten_div = 10 / _;
            REQUIRE(ten_div(2) == 5);
        }
    }
    ///
    SECTION("operator %") {
        auto const mod = _ % _;
        REQUIRE(mod(7, 2) == 1);
    }
    SECTION("partial operator %") {
        {
            auto const mod_2 = _ % 2;
            REQUIRE(mod_2(3) == 1);
        }
        {
            auto const ten_mod = 10 % _;
            REQUIRE(ten_mod(3) == 1);
        }
    }
    ///
    SECTION("operator ==") {
        auto const eq = _ == _;
        REQUIRE_FALSE(eq(7, 2));
        REQUIRE(eq(2, 2));
    }
    SECTION("partial operator ==") {
        {
            auto const eq_2 = _ == 2;
            REQUIRE(eq_2(2));
            REQUIRE_FALSE(eq_2(1));
        }
        {
            auto const eq_2 = 2 == _;
            REQUIRE(eq_2(2));
            REQUIRE_FALSE(eq_2(1));
        }
    }
    ///
    SECTION("operator !=") {
        auto const neq = _ != _;
        REQUIRE(neq(7, 2));
        REQUIRE_FALSE(neq(2, 2));
    }
    SECTION("partial operator !=") {
        {
            auto const neq_2 = _ != 2;
            REQUIRE_FALSE(neq_2(2));
            REQUIRE(neq_2(1));
        }
        {
            auto const neq_2 = 2 != _;
            REQUIRE_FALSE(neq_2(2));
            REQUIRE(neq_2(1));
        }
    }
    ///
    SECTION("operator >") {
        auto const gt = _ > _;
        REQUIRE(gt(7, 2));
        REQUIRE_FALSE(gt(2, 2));
    }
    SECTION("partial operator >") {
        {
            auto const gt_2 = _ > 2;
            REQUIRE_FALSE(gt_2(2));
            REQUIRE(gt_2(3));
        }
        {
            auto const lt_2 = 2 > _;
            REQUIRE_FALSE(lt_2(2));
            REQUIRE(lt_2(1));
        }
    }
    ///
    SECTION("operator >=") {
        auto const ge = _ >= _;
        REQUIRE(ge(7, 2));
        REQUIRE(ge(2, 2));
        REQUIRE_FALSE(ge(1, 2));
    }
    SECTION("partial operator >=") {
        {
            auto const ge_2 = _ >= 2;
            REQUIRE(ge_2(2));
            REQUIRE(ge_2(3));
            REQUIRE_FALSE(ge_2(1));
        }
        {
            auto const le_2 = 2 >= _;
            REQUIRE(le_2(2));
            REQUIRE(le_2(1));
            REQUIRE_FALSE(le_2(3));
        }
    }
    ///
    SECTION("operator <") {
        auto const lt = _ < _;
        REQUIRE_FALSE(lt(7, 2));
        REQUIRE(lt(1, 2));
    }
    SECTION("partial operator <") {
        {
            auto const lt_2 = _ < 2;
            REQUIRE_FALSE(lt_2(2));
            REQUIRE(lt_2(1));
        }
        {
            auto const gt_2 = 2 < _;
            REQUIRE_FALSE(gt_2(2));
            REQUIRE(gt_2(3));
        }
    }
    ///
    SECTION("operator <=") {
        auto const le = _ <= _;
        REQUIRE_FALSE(le(7, 2));
        REQUIRE(le(2, 2));
        REQUIRE(le(1, 2));
    }
    SECTION("partial operator <=") {
        {
            auto const le_2 = _ <= 2;
            REQUIRE(le_2(2));
            REQUIRE_FALSE(le_2(3));
            REQUIRE(le_2(1));
        }
        {
            auto const ge_2 = 2 <= _;
            REQUIRE(ge_2(2));
            REQUIRE_FALSE(ge_2(1));
            REQUIRE(ge_2(3));
        }
    }
    ///
    SECTION("operator !") {
        auto const neg = !_;
        REQUIRE(neg(false));
        REQUIRE_FALSE(neg(true));
    }
    ///
    SECTION("operator &&") {
        auto const and_ = _ && _;
        REQUIRE(and_(true, true));
        REQUIRE_FALSE(and_(true, false));
    }
    SECTION("partial operator &&") {
        {
            auto const and_true = _ && true;
            REQUIRE(and_true(true));
            REQUIRE_FALSE(and_true(false));
        }
        {
            auto const and_true = true && _;
            REQUIRE(and_true(true));
            REQUIRE_FALSE(and_true(false));
        }
    }
    ///
    SECTION("operator ||") {
        auto const or_ = _ || _;
        REQUIRE(or_(false, true));
        REQUIRE_FALSE(or_(false, false));
    }
    SECTION("partial operator ||") {
        {
            auto const or_false = _ || false;
            REQUIRE(or_false(true));
            REQUIRE_FALSE(or_false(false));
        }
        {
            auto const true_or = true || _;
            REQUIRE(true_or(true));
            REQUIRE(true_or(false));
        }
    }
    ///
    SECTION("operator ~") {
        auto const neg = ~_;
        REQUIRE(neg(2U) == ~2U);
    }
    ///
    SECTION("operator &") {
        auto const and_ = _ & _;
        REQUIRE(and_(5U, 3U) == (5U & 3U));
    }
    SECTION("partial operator &") {
        {
            auto const and_3 = _ & 3U;
            REQUIRE(and_3(2U) == (2U & 3U));
        }
        {
            auto const three_and = 3U & _;
            REQUIRE(three_and(2U) == (3U & 2U));
        }
    }
    ///
    SECTION("operator |") {
        auto const or_ = _ | _;
        REQUIRE(or_(5U, 3U) == (5U | 3U));
    }
    SECTION("partial operator |") {
        {
            auto const or_3 = _ | 3U;
            REQUIRE(or_3(2U) == (2U | 3U));
        }
        {
            auto const three_or = 3U | _;
            REQUIRE(three_or(2U) == (3U | 2U));
        }
    }
    ///
    SECTION("operator ^") {
        auto const xor_ = _ ^ _;
        REQUIRE(xor_(5U, 3U) == (5U ^ 3U));
    }
    SECTION("partial operator ^") {
        {
            auto const xor_3 = _ ^ 3U;
            REQUIRE(xor_3(2U) == (2U ^ 3U));
        }
        {
            auto const three_xor = 3U ^ _;
            REQUIRE(three_xor(2U) == (3U ^ 2U));
        }
    }
    SECTION("fn") {
        auto const getter = _.fn(&Obj::get);
        REQUIRE(getter(Obj{}) == Obj{}.get());
    }
    SECTION("fn and wildcard") {
        {
            auto const greater_than_40 = _.fn(&Obj::get) > 40;
            REQUIRE(greater_than_40(Obj{}));
        }
        {
            auto const equal = _.fn(&Obj::get) == Obj{}.get();
            REQUIRE(equal(Obj{}));
        }
        {
            auto const less_than_40 = _.fn(&Obj::get) < 40;
            REQUIRE_FALSE(less_than_40(Obj{}));
        }
        {
            auto const is_greater = _.fn(&Obj::get) > _.fn(&Obj::get);
            REQUIRE(is_greater(Obj{10}, Obj{1}));
        }
        {
            auto const is_greater = _.fn(&Obj::get) > _;
            REQUIRE(is_greater(Obj{10}, 1));
        }
    }
    SECTION("fn and wildcard chained") {
        auto const square_greater_than = _.fn(&Obj::square).fn(&Obj::get) > _;
        REQUIRE_FALSE(square_greater_than(Obj{1}, 4));
        REQUIRE(square_greater_than(Obj{3}, 4));
    }
    SECTION("fn and wildcard macros") {
        {
            auto const value_greater_than = _.fn(CALL(get)) > _;
            REQUIRE_FALSE(value_greater_than(Obj{1}, 4));
            REQUIRE(value_greater_than(Obj{5}, 4));
        }
        {
            auto const x = 2;
            auto const equal = _.fn(CALL_C(mult, x)) == _;
            REQUIRE_FALSE(equal(Obj{1}, 3));
            REQUIRE(equal(Obj{2}, 4));
        }
        {
            auto const x = 2;
            auto const equal = _.fn(CALL_R(mult, x)) == _;
            REQUIRE_FALSE(equal(Obj{1}, 3));
            REQUIRE(equal(Obj{2}, 4));
        }
    }
}
