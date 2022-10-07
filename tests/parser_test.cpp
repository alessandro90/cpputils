#include "cpputils/misc/parsers.hpp"
#include "cpputils/misc/visitor.hpp"
#include <catch2/catch_all.hpp>

#include <iostream>

using namespace cpputils;
using namespace cpputils::parsers;

namespace {
void print_(json::Object const &o);
void print_(json::Null const &) { std::cout << "Null, "; }
void print_(json::Bool const &primitive) { std::cout << primitive << ", "; };
void print_(json::String const &primitive) { std::cout << primitive << ", "; };
void print_(json::Number const &primitive) { std::cout << primitive << ", "; };
void print_(json::Array const &a) {
    std::cout << "[ \n";
    for (auto const &item : a) {
        std::visit([](auto const &x) { print_(x); }, item.json_value);
    }
    std::cout << "\n]\n";
}

void print_(json::Object const &o) {
    for (auto const &item : o) {
        std::cout << item->key << ": {\n";
        std::visit([](auto const &x) { print_(x); }, item->value.json_value);
        std::cout << "}\n";
    }
}

void print_json(json::Object const &o) {
    print_(o);
}
}  // namespace


TEST_CASE("parser-test-stub", "[check-compilation]") {
    const char *json_text =
        R"xx( { "something": { "v_0": 10, "r": { "v_1": [1, 2] } }, "something_else": 1245.3 } )xx";

    auto const parsed = json::parse(json_text);
    if (!parsed) {
        std::cout << "Invalid json\n";
    } else {
        auto const &obj = parsed.value().m_json_object;
        std::cout << "JSON level 0 size -> " << obj.size() << '\n';
        print_json(obj);
        std::cout << "\n";
    }
    SUCCEED();
}