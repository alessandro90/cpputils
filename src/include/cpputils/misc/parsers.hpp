#ifndef CPPUTILS_PARSERS_HPP
#define CPPUTILS_PARSERS_HPP

#include <charconv>
#include <concepts>
#include <cstddef>
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

    [[nodiscard]] inline bool is_digit(char ch) { return std::isdigit(ch) != 0; }

    [[nodiscard]] inline bool is_number_begin(std::string_view fcontent) {
        if (fcontent.empty()) { return false; }
        if (is_digit(fcontent[0])) { return true; }
        return fcontent.size() > 1 && fcontent[0] == '-' && is_digit(fcontent[1]);
    }

    template <std::floating_point NumType>
    [[nodiscard]] std::optional<NumType> sv_to_number(std::string_view v) {
        NumType n{};
        auto const conversion_result = std::from_chars(v.begin(), v.end(), n);
        if (conversion_result.ptr == v.end()) { return n; }
        return std::nullopt;
    }
}  // namespace detail

class json {
private:
    struct key_and_value;
    using object_representation = std::vector<key_and_value>;

public:
    struct Null {};
    using Bool = bool;
    using String = std::string_view;
    using Number = double;
    using Object = std::unique_ptr<key_and_value>;
    using Array = std::vector<Object>;

    struct parsed_json {
        object_representation m_object_representation;
    };

    [[nodiscard]] static std::optional<parsed_json> parse(std::string_view fcontent) {
        object_representation json_object{};
        auto to_parse = fcontent;
        while (!fcontent.empty()) {
            // TODO: A for loop for all the parsers here
            auto const key_and_to_parse = split_key_and_remaining(to_parse);
            if (!key_and_to_parse) { return std::nullopt; }
            auto const [key, to_parse_after_key] = key_and_to_parse.value();
            auto parse_result = parse_object(to_parse_after_key);
            if (!parse_result) { return std::nullopt; }  // Invalid json
            auto &[value, to_parse_after_value] = parse_result.value();
            to_parse = to_parse_after_value;
            if (!value) { continue; }
            json_object.emplace_back(key, std::move(value).value());
        }
        return parsed_json{std::move(json_object)};
    }
    // For debug of single parsers
    /*
    std::optional<key_and_value> parse(std::string_view fcontent) {
        auto to_parse = fcontent;
        auto const km = split_key_and_remaining(to_parse);
        if (!km) { return {}; }
        auto const [key, to_parse_after_key] = km.value();
        auto parse_result = parse_null(to_parse_after_key);
        if (!parse_result) { return {}; }  // Invalid json
        auto &[value, to_parse_after_value] = parse_result.value();
        to_parse = to_parse_after_value;
        if (!value) { return {}; }
        return key_and_value{key, std::move(value).value()};
    }
    */

private:
    using json_value = std::variant<Null, Bool, String, Number, Array, Object>;

    // NOLINTNEXTLINE
    struct key_and_value {
        std::string_view key{};
        json_value value{};
    };

    // NOLINTNEXTLINE
    struct parse_result {
        std::optional<json_value> value{};
        std::string_view remaining{};
    };

    struct key_and_remaining {
        std::string_view key;
        std::string_view remaining;
    };

    [[nodiscard]] static bool is_record_end(char ch) noexcept {
        return detail::is_newline(ch) || ch == ',' || ch == ' ' || ch == '}';
    }

    [[nodiscard]] static std::optional<std::string_view> strip_marks(std::string_view v) {
        if (v.size() < 3 || !v.starts_with('"') || !v.ends_with('"')) { return std::nullopt; }
        v.remove_prefix(1);
        v.remove_suffix(1);
        return v;
    }

    [[nodiscard]] static std::optional<key_and_remaining> split_key_and_remaining(std::string_view fcontent) {
        auto const colon_pos = fcontent.find(':');
        if (colon_pos == std::string_view::npos) { return std::nullopt; }
        auto const maybe_key = strip_marks(detail::strip(fcontent.substr(0, colon_pos)));
        if (!maybe_key) { return std::nullopt; }
        auto const key = maybe_key.value();
        if (key.size() + 1U == fcontent.size()) { return std::nullopt; }
        auto const remaining = detail::strip(fcontent.substr(colon_pos + 1U));
        return key_and_remaining{.key = key, .remaining = remaining};
    }

    [[nodiscard]] static std::optional<parse_result> parse_keyword(std::string_view fcontent, std::string_view keyword, auto keyword_value) {  // NOLINT
        if (fcontent.size() < keyword.size() + 1 || fcontent.substr(0, keyword.size()) != keyword) {
            return parse_result{.value = std::nullopt, .remaining = fcontent};
        }
        if (!is_record_end(fcontent[keyword.size()])) {
            return parse_result{.value = std::nullopt, .remaining = fcontent};
        }
        return parse_result{.value = json_value{keyword_value}, .remaining = fcontent.substr(keyword.size())};
    }

    [[nodiscard]] static std::optional<parse_result> parse_object(std::string_view) {
        return {};
    }

    [[nodiscard]] static std::optional<parse_result> parse_null(std::string_view fcontent) {
        return parse_keyword(fcontent, "null", Null{});
    }

    [[nodiscard]] static std::optional<parse_result> parse_bool(std::string_view fcontent) {
        if (auto parse_true = parse_keyword(fcontent, "true", true);
            !parse_true || parse_true.value().value) {
            return parse_true;
        }
        return parse_keyword(fcontent, "false", false);
    }

    [[nodiscard]] static std::optional<parse_result> parse_string(std::string_view fcontent) {
        if (fcontent.empty()) { return std::nullopt; }
        if (fcontent[0] != '"') { return parse_result{.value = std::nullopt, .remaining = fcontent}; }
        if (auto const closing_mark = fcontent.find_first_of('"', 1); closing_mark != std::string_view::npos) {
            return parse_result{.value = json_value{fcontent.substr(1, closing_mark - 1U)},  // Skip marks
                                .remaining = fcontent.substr(closing_mark + 1U)};
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<parse_result> parse_number(std::string_view fcontent) {
        if (fcontent.empty() || !detail::is_number_begin(fcontent)) { return std::nullopt; }
        bool found_dot{false};
        std::size_t ch_pos{};
        bool const is_negative = fcontent[0] == '-';
        if (is_negative) { ++ch_pos; }
        while (ch_pos < fcontent.size()) {
            auto const ch = fcontent[ch_pos];
            if (detail::is_digit(ch)) {
                ++ch_pos;
                continue;
            }
            if (ch_pos > 0 && ch == '.' && !found_dot) {
                found_dot = true;
                ++ch_pos;
                continue;
            }
            bool const is_end = is_record_end(ch);
            if ((!is_end && !detail::is_digit(ch)) || (found_dot && ch == '.')) { return std::nullopt; }
            if (is_end) { break; }
        }
        if (ch_pos == 0 || (is_negative && ch_pos == 1)) { return std::nullopt; }

        return parse_result{.value = detail::sv_to_number<Number>(fcontent.substr(0, ch_pos)).value_or(0.0),
                            .remaining = fcontent.substr(ch_pos)};
    }

    [[nodiscard]] static std::optional<parse_result> parse_array(std::string_view) {
        return {};
    }
};
}  // namespace cpputils::parsers

#endif
