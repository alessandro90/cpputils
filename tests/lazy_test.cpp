#include "cpputils/types/lazy.hpp"
#include <catch2/catch_all.hpp>

#include <any>

using namespace cpputils;

namespace {
struct ExpensiveBuild_1 {
    int a, b, c;
};

class ExpensiveBuild_2 {
public:
    explicit ExpensiveBuild_2(int a, int b, int c)
        : m_a{a}
        , m_b{b}
        , m_c{c} {}

    int get_a() const { return m_a; }
    int get_b() const { return m_b; }
    int get_c() const { return m_c; }

private:
    int m_a, m_b, m_c;
};

class ExpensiveBuild_3 {
public:
    explicit ExpensiveBuild_3(int a)
        : m_a{a} {}

    explicit ExpensiveBuild_3(int a, int b)
        : m_a{a}
        , m_b{b} {}

    explicit ExpensiveBuild_3(int a, int b, int c)
        : m_a{a}
        , m_b{b}
        , m_c{c} {}

    int get_a() const { return m_a; }
    int get_b() const { return m_b; }
    int get_c() const { return m_c; }

private:
    int m_a, m_b{}, m_c{};
};

struct CustomBuilder_ExpensiveBuild_3 {
    template <builder_for<ExpensiveBuild_3> B>
    explicit CustomBuilder_ExpensiveBuild_3(B b)
        : internal_builder{b}
        , builder{[](std::any const *bld) { return (*std::any_cast<B>(bld))(); }} {}

    ExpensiveBuild_3 operator()() const { return builder(&internal_builder); }

    std::any internal_builder;
    ExpensiveBuild_3 (*builder)(std::any const *);
};

}  // namespace

// NOLINTNEXTLINE
TEST_CASE("lazy-tests", "[check lazy]") {
    SECTION("lazy aggregate") {
        {
            auto const lazy_aggregate = lazy<ExpensiveBuild_1>(10, 20, 30);
            auto const expensive_build_1{lazy_aggregate()};
            REQUIRE(expensive_build_1.a == 10);
            REQUIRE(expensive_build_1.b == 20);
            REQUIRE(expensive_build_1.c == 30);
        }
        {
            auto const lazy_aggregate = lazy<ExpensiveBuild_1>();
            auto const expensive_build_1{lazy_aggregate()};
            REQUIRE(expensive_build_1.a == 0);
            REQUIRE(expensive_build_1.b == 0);
            REQUIRE(expensive_build_1.c == 0);
        }
    }
    SECTION("lazy class single constructor") {
        auto const lz = lazy<ExpensiveBuild_2>(10, 20, 30);
        auto const expensive_build_2{lz()};
        REQUIRE(expensive_build_2.get_a() == 10);
        REQUIRE(expensive_build_2.get_b() == 20);
        REQUIRE(expensive_build_2.get_c() == 30);
    }
    SECTION("lazy class multiple constructors") {
        {
            auto const lz = lazy<ExpensiveBuild_3>(10);
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 0);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            auto const lz = lazy<ExpensiveBuild_3>(10, 20);
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            auto const lz = lazy<ExpensiveBuild_3>(10, 20, 30);
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 30);
        }
    }
}

// NOLINTNEXTLINE
TEST_CASE("Lazy-tests", "[check Lazy]") {
    SECTION("Lazy aggregate") {
        {
            Lazy<ExpensiveBuild_1> const lazy_aggregate{10, 20, 30};
            auto const expensive_build_1{lazy_aggregate()};
            REQUIRE(expensive_build_1.a == 10);
            REQUIRE(expensive_build_1.b == 20);
            REQUIRE(expensive_build_1.c == 30);
        }
        {
            Lazy<ExpensiveBuild_1> const lazy_aggregate{};
            auto const expensive_build_1{lazy_aggregate()};
            REQUIRE(expensive_build_1.a == 0);
            REQUIRE(expensive_build_1.b == 0);
            REQUIRE(expensive_build_1.c == 0);
        }
    }
    SECTION("lazy class single constructor") {
        Lazy<ExpensiveBuild_2> const lz{10, 20, 30};
        auto const expensive_build_2{lz()};
        REQUIRE(expensive_build_2.get_a() == 10);
        REQUIRE(expensive_build_2.get_b() == 20);
        REQUIRE(expensive_build_2.get_c() == 30);
    }
    SECTION("lazy class multiple constructors") {
        {
            Lazy<ExpensiveBuild_3> const lz{10};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 0);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            Lazy<ExpensiveBuild_3> const lz{10, 20};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            Lazy<ExpensiveBuild_3> const lz{10, 20, 30};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 30);
        }
    }
    SECTION("check Lazy with custom builder") {
        using LazyExpensiveBuild_3 = Lazy<ExpensiveBuild_3, CustomBuilder_ExpensiveBuild_3>;
        {
            LazyExpensiveBuild_3 const lz{10};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 0);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            LazyExpensiveBuild_3 const lz{10, 20};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 0);
        }
        {
            LazyExpensiveBuild_3 const lz{10, 20, 30};
            auto const expensive_build_3{lz()};
            REQUIRE(expensive_build_3.get_a() == 10);
            REQUIRE(expensive_build_3.get_b() == 20);
            REQUIRE(expensive_build_3.get_c() == 30);
        }
    }
}
