#ifndef CPPUTILS_PARSERS_HPP
#define CPPUTILS_PARSERS_HPP

#include <cstddef>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


namespace cpputils::parsers {
namespace detail {
    [[nodiscard]] inline constexpr bool is_space(char ch) noexcept {
        return ch == ' ' || ch == '\t';
    }

    [[nodiscard]] inline bool is_newline(char ch) noexcept { return ch == '\n' || ch == '\r'; }

    [[nodiscard]] inline std::string_view skip_newline(std::string_view s) {
        if (s.empty()) { return s; }
        return s.substr(
            static_cast<std::size_t>(s[0] == '\n')
            + static_cast<std::size_t>(s[0] == '\r')
            + static_cast<std::size_t>(s[1] == '\r'));
    }

    [[nodiscard]] inline std::string_view skip_comma(std::string_view s) {
        if (s.empty() || s[0] != ',') { return s; }
        return s.substr(1);
    }

    [[nodiscard]] inline std::string_view lstrip(std::string_view fcontent) {
        std::size_t current{};
        while (current < fcontent.size() && is_space(fcontent[current])) {
            ++current;
        }
        return fcontent.substr(current);
    }

    [[nodiscard]] inline std::string_view rstrip(std::string_view fcontent) {
        if (fcontent.empty()) { return fcontent; }
        std::size_t current{fcontent.size()};
        while (current + 1U > 0 && is_space(fcontent[current])) {
            --current;
        }
        return fcontent.substr(0U, current);
    }
    [[nodiscard]] inline std::string_view strip(std::string_view fcontent) {
        return rstrip(lstrip(fcontent));
    }
}  // namespace detail

class json {
private:
    struct object_representation;
    // struct key_and_value;
public:
    struct Null {};
    using Bool = bool;
    using String = std::string;
    using Number = double;
    using Object = std::unique_ptr<object_representation>;
    // using Object = std::list<key_and_value>;
    using Array = std::vector<Object>;

    bool parse(std::string_view fcontent) {
        m_json_object = make_object();
        object_representation *object_being_parsed = m_json_object.get();
        while (!fcontent.empty()) {
            // TODO: A for loop for all the parsers here
            auto [key_value, remaining] = parse_object(fcontent);
            fcontent = remaining;
            if (!key_value) { continue; }
            // m_json_object.push_back(std::move(key_value));
            object_being_parsed->key_value = std::move(key_value);
            object_being_parsed->next_object = make_object();
            object_being_parsed = object_being_parsed->next_object.get();
        }
        return true;
    }

private:
    Object m_json_object{};

    using json_value = std::variant<Null, Bool, String, Number, Array, Object>;

    // NOLINTNEXTLINE
    struct key_and_value {
        std::string_view key{};
        json_value value{};
    };

    // TODO: what about using a std::list here?
    // using object_representation = std::list<key_and_value>;
    struct object_representation {
        std::optional<key_and_value> key_value{};
        Object next_object{};
    };

    // NOLINTNEXTLINE
    struct parse_result {
        std::optional<key_and_value> key_value{};
        std::string_view remaining{};
    };

    struct key_and_remaining {
        std::string_view key;
        std::string_view remaining;
    };

    [[nodiscard]] static Object make_object() { return std::make_unique<object_representation>(); }

    [[nodiscard]] static bool is_record_end(char ch) noexcept { return detail::is_newline(ch) || ch == ','; }

    [[nodiscard]] static std::optional<key_and_remaining> split_key_and_remaining(std::string_view fcontent) {
        auto const colon_pos = fcontent.find(':');
        if (colon_pos == std::string_view::npos) { return std::nullopt; }
        auto const key = fcontent.substr(0, colon_pos);
        if (key.size() + 1U == fcontent.size()) { return std::nullopt; }
        auto const remaining = fcontent.substr(colon_pos + 1U);
        return key_and_remaining{.key = detail::strip(key), .remaining = detail::strip(remaining)};
    }

    [[nodiscard]] static parse_result parse_keyword(std::string_view fcontent, std::string_view keyword, auto keyword_value) {  // NOLINT
        auto const km = split_key_and_remaining(fcontent);
        if (!km) { return {.key_value = std::nullopt, .remaining = fcontent}; }
        auto const [key, remaining] = km.value();
        if (remaining.size() < keyword.size() + 1 || remaining.substr(0, keyword.size()) != keyword) {
            return {.key_value = std::nullopt, .remaining = fcontent};
        }
        if (!is_record_end(remaining[keyword.size()])) {
            return {.key_value = std::nullopt, .remaining = fcontent};
        }
        return {.key_value = key_and_value{.key = key, .value = keyword_value},
                .remaining = remaining.substr(keyword.size())};
    }

    // TODO: could return an optional<parse_result> to indicate invalid json format
    [[nodiscard]] static parse_result parse_object(std::string_view) {
        return {};
    }

    [[nodiscard]] static parse_result parse_null(std::string_view fcontent) {
        return parse_keyword(fcontent, "null", Null{});
    }

    [[nodiscard]] static parse_result parse_bool(std::string_view fcontent) {
        if (auto parse_true = parse_keyword(fcontent, "true", true); parse_true.key_value) {
            return parse_true;
        }
        return parse_keyword(fcontent, "false", false);
    }

    [[nodiscard]] static parse_result parse_string(std::string_view) {
        return {};
    }

    [[nodiscard]] static parse_result parse_number(std::string_view) {
        return {};
    }

    [[nodiscard]] static parse_result parse_array(std::string_view) {
        return {};
    }
};
}  // namespace cpputils::parsers

#endif
