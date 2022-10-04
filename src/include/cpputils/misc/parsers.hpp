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
    [[nodiscard]] constexpr bool char_is(char ch, auto... chars) noexcept
        requires std::conjunction_v<std::is_same<char, decltype(chars)>...>
    {
        return ((ch == chars) || ...);
    }

    [[nodiscard]] inline constexpr bool is_space(char ch) noexcept {
        return char_is(ch, ' ', '\t');
    }

    [[nodiscard]] inline bool is_newline(char ch) noexcept {
        return char_is(ch, '\n', '\r');
    }

    [[nodiscard]] inline std::string_view skip_newline(std::string_view s) noexcept {
        if (s.empty()) { return s; }
        return s.substr(static_cast<std::size_t>(s[0] == '\n')
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
        if (auto const conv = std::from_chars(v.begin(), v.end(), n); conv.ptr == v.end()) {
            return n;
        }
        return std::nullopt;
    }
}  // namespace detail

class json {
private:
    struct key_and_value_t;
    using json_representation_t = std::vector<key_and_value_t>;

public:
    struct Null {};
    using Bool = bool;
    using String = std::string_view;
    using Number = double;
    using Object = std::unique_ptr<key_and_value_t>;
    using Array = std::vector<Object>;

    struct parsed_json_t {
        json_representation_t m_object_representation;
    };

    [[nodiscard]] static std::optional<parsed_json_t> parse(std::string_view fcontent) {
        if (fcontent.empty()) { return std::nullopt; }
        json_representation_t json_object{};
        auto to_parse = detail::lstrip(detail::skip_newline(skip_brace(detail::lstrip(fcontent), '{')));
        while (!to_parse.empty() && to_parse[0] != '}') {
            auto parse_output = traverse_parsers(to_parse);
            if (!parse_output) { return std::nullopt; }
            auto &[parsed_object, remaining_chars] = parse_output.value();
            to_parse = detail::lstrip(remaining_chars);
            json_object.push_back(std::move(parsed_object));
        }
        if (to_parse.empty() || to_parse[0] != '}') { return std::nullopt; }
        return parsed_json_t{std::move(json_object)};
    }

private:
    using json_value_t = std::variant<Null, Bool, String, Number, Array, Object>;

    // NOLINTNEXTLINE
    struct key_and_value_t {
        std::string_view key{};
        json_value_t value{};
    };

    // NOLINTNEXTLINE
    struct parse_result_t {
        std::optional<json_value_t> value{};
        std::string_view remaining{};
    };

    struct key_and_remaining_t {
        std::string_view key;
        std::string_view remaining;
    };

    struct parsers_traversal_output_t {
        key_and_value_t key_value{};
        std::string_view remaining{};
    };

    [[nodiscard]] static bool is_record_end(char ch) noexcept {
        return detail::is_newline(ch) || detail::char_is(ch, ',', ' ', '}');
    }

    [[nodiscard]] static std::optional<std::string_view> strip_marks(std::string_view v) {
        if (v.size() < 3 || !v.starts_with('"') || !v.ends_with('"')) { return std::nullopt; }
        v.remove_prefix(1);
        v.remove_suffix(1);
        return v;
    }

    [[nodiscard]] static std::string_view skip_brace(std::string_view s, char brace) {
        if (!s.empty() && s[0] == brace) { s.remove_prefix(1); }
        return s;
    }

    [[nodiscard]] static std::optional<key_and_remaining_t> split_key_and_remaining(std::string_view fcontent) {
        auto const colon_pos = fcontent.find(':');
        if (colon_pos == std::string_view::npos) { return std::nullopt; }
        auto const maybe_key = strip_marks(detail::strip(fcontent.substr(0, colon_pos)));
        if (!maybe_key) { return std::nullopt; }
        auto const key = maybe_key.value();
        if (key.size() + 1U == fcontent.size()) { return std::nullopt; }
        auto const remaining = detail::lstrip(fcontent.substr(colon_pos + 1U));
        if (remaining.empty()) { return std::nullopt; }
        return key_and_remaining_t{.key = key, .remaining = remaining};
    }

    [[nodiscard]] static std::optional<parsers_traversal_output_t> traverse_parsers(std::string_view fcontent) {
        auto to_parse = fcontent;
        auto const key_and_to_parse = split_key_and_remaining(to_parse);
        if (!key_and_to_parse) { return std::nullopt; }
        auto const [key, to_parse_after_key] = key_and_to_parse.value();
        for (auto parser : s_parsers) {
            auto parse_result = parser(to_parse_after_key);
            if (!parse_result) { return std::nullopt; }  // Invalid json
            auto &[value, to_parse_after_value] = parse_result.value();
            to_parse = to_parse_after_value;
            if (!value) { continue; }  // Current parser does not match, try the next one
            return parsers_traversal_output_t{.key_value = {.key = key, .value = std::move(value).value()}, .remaining = to_parse};
        }
        // No parser could interpret the json -> invalid json
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_keyword(std::string_view fcontent, std::string_view keyword, auto keyword_value) {  // NOLINT
        if (fcontent.size() < keyword.size() + 1 || fcontent.substr(0, keyword.size()) != keyword || !is_record_end(fcontent[keyword.size()])) {
            return parse_result_t{.value = std::nullopt, .remaining = fcontent};
        }
        return parse_result_t{.value = json_value_t{keyword_value},
                              .remaining = detail::skip_newline(detail::skip_comma(fcontent.substr(keyword.size())))};
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_object(std::string_view fcontent) {
        // fcontent = detail::lstrip(fcontent);
        if (fcontent[0] != '{') { return parse_result_t{.value = std::nullopt, .remaining = fcontent}; }

        fcontent = detail::lstrip(detail::skip_newline(skip_brace(fcontent, '{')));

        auto parsed = traverse_parsers(fcontent);
        if (!parsed) { return std::nullopt; }

        auto &[key_value, remaining] = parsed.value();

        return parse_result_t{.value = std::make_unique<key_and_value_t>(std::move(key_value)),
                              .remaining = detail::skip_newline(skip_brace(detail::lstrip(remaining), '}'))};
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_null(std::string_view fcontent) {
        return parse_keyword(fcontent, "null", Null{});
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_bool(std::string_view fcontent) {
        if (auto parse_true = parse_keyword(fcontent, "true", true);
            !parse_true || parse_true.value().value) {
            return parse_true;
        }
        return parse_keyword(fcontent, "false", false);
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_string(std::string_view fcontent) {
        if (fcontent[0] != '"') { return parse_result_t{.value = std::nullopt, .remaining = fcontent}; }
        if (auto const closing_mark = fcontent.find_first_of('"', 1); closing_mark != std::string_view::npos) {
            return parse_result_t{.value = json_value_t{fcontent.substr(1, closing_mark - 1U)},  // Skip marks
                                  .remaining = detail::skip_newline(detail::skip_comma(fcontent.substr(closing_mark + 1U)))};
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_number(std::string_view fcontent) {
        if (!detail::is_number_begin(fcontent)) {
            return std::nullopt;
        }
        bool const is_negative = fcontent[0] == '-';
        std::size_t ch_pos{};
        if (is_negative) { ++ch_pos; }
        bool found_dot{false};
        while (ch_pos < fcontent.size()) {
            auto const ch = fcontent[ch_pos];
            if (detail::is_digit(ch)) {
                ++ch_pos;
                continue;
            }
            bool const is_dot = ch == '.';
            if (ch_pos > 0 && is_dot && !found_dot) {
                found_dot = true;
                ++ch_pos;
                continue;
            }
            bool const is_end = is_record_end(ch);
            if ((!is_end && !detail::is_digit(ch)) || (found_dot && is_dot)) { return std::nullopt; }
            if (is_end) { break; }
        }
        if (ch_pos == 0 || (is_negative && ch_pos == 1)) { return std::nullopt; }

        return parse_result_t{.value = detail::sv_to_number<Number>(fcontent.substr(0, ch_pos)).value_or(0.0),
                              .remaining = detail::skip_newline(detail::skip_comma(fcontent.substr(ch_pos)))};
    }

    [[nodiscard]] static std::optional<parse_result_t> parse_array(std::string_view) {
        return {};
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    inline static constexpr std::optional<parse_result_t> (*s_parsers[])(std::string_view) =
        {&json::parse_object,
         &json::parse_null,
         &json::parse_bool,
         &json::parse_string,
         &json::parse_number,
         &json::parse_array};
};
}  // namespace cpputils::parsers

#endif