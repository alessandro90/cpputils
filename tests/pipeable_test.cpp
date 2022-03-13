#include "catch2/catch_test_macros.hpp"
#include "cpputils/functional/enumerate.hpp"
#include "cpputils/functional/pipeable.hpp"
#include "cpputils/functional/zip.hpp"
#include "cpputils/functional/zip_with.hpp"
#include <ranges>

using namespace cpputils;
namespace vs = std::views;

TEST_CASE("pipebale test", "[pipe before]") {  // NOLINT
    [[maybe_unused]] auto const op = pipeable(vs::filter([](int) { return true; })) | enumerate();
    SUCCEED();
}

TEST_CASE("pipebale test", "[pipe after]") {  // NOLINT
    [[maybe_unused]] auto const op = enumerate() | pipeable(vs::filter([](int) { return true; }));
    SUCCEED();
}
