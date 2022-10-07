#include "cpputils/misc/parsers.hpp"
#include "cpputils/misc/visitor.hpp"
#include <catch2/catch_all.hpp>

#include <iostream>

using namespace cpputils;
using namespace cpputils::parsers;

namespace {
void print_object(json::json_value_t::alternatives_t const &obj) {
    visitor const v{
        [](json::Null) { std::cout << "Null, "; },
        [](json::Object const &o) {
            std::cout << "{ \n";
            std::cout << o->key << ": ";
            print_object(o->value.json_value);
            std::cout << " }\n";
        },
        [](json::Array const &a) {
            std::cout << "[ \n";
            for (auto const &item : a) {
                print_object(item.json_value);
            }
            std::cout << "\n]\n";
        },
        [](auto primitive) { std::cout << primitive << ", "; },
    };
    std::visit(v, obj);
}
}  // namespace


TEST_CASE("parser-test-stub", "[check-compilation]") {
    const char *json_text =
        R"xx( { "something": { "v_0": 10, "r": { "v_1": [1, 2] } } } )xx";

    auto const parsed = json::parse(json_text);
    if (!parsed) {
        std::cout << "Invalid json\n";
    } else {
        auto const &obj = parsed.value().m_object_representation;
        std::cout << "JSON level 0 size -> " << obj.size() << '\n';
        for (auto const &obj_elem : obj) {
            auto const &[key, value] = obj_elem;
            std::cout << "Key -> \n";
            std::cout << key << '\n';
            auto const &json_value = value.json_value;
            std::cout << "value is -> \n";
            print_object(json_value);
            std::cout << "\n";
        }
    }
    SUCCEED();
}