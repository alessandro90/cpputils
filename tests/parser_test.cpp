#include "cpputils/misc/parsers.hpp"
#include <catch2/catch_all.hpp>

#include <iostream>

using namespace cpputils::parsers;


TEST_CASE("parser-test-stub", "[check-compilation]") {
    const char *json_text =
        R"xx( { "something": "some string" } )xx";

    auto const parsed = json::parse(json_text);
    if (!parsed) {
        std::cout << "Invalid json\n";
    } else {
        auto const &obj = parsed.value().m_object_representation;
        std::cout << "JSON level 0 size -> " << obj.size() << '\n';
        auto const &[key, value] = obj[0];
        std::cout << "Key -> \n";
        std::cout << key << '\n';

        std::cout << "value is -> \n";
        if (std::holds_alternative<json::Null>(value)) {
            std::cout << "null" << '\n';
        }
        if (std::holds_alternative<json::String>(value)) {
            std::cout << std::get<json::String>(value) << '\n';
        }
        if (std::holds_alternative<json::Number>(value)) {
            std::cout << std::get<json::Number>(value) << '\n';
        }
    }
    SUCCEED();
}