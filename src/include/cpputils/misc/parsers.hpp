#ifndef CPPUTILS_PARSERS_HPP
#define CPPUTILS_PARSERS_HPP

// #include <iostream>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


namespace cpputils::parsers {
namespace parsers_impl {
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

    // [[nodiscard]] inline std::string_view skip_newlines_and_spaces(std::string_view s) {
    //     if (s.empty()) { return s; }
    //     std::size_t count{};
    //     while (is_newline(s[count]) || is_space(s[count])) { ++count; }
    //     return s.substr(count);
    // }

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

    [[nodiscard]] static std::string_view skip_brace(std::string_view s, char brace) {
        if (!s.empty() && s[0] == brace) { s.remove_prefix(1); }
        return s;
    }


    [[nodiscard]] inline std::optional<std::string_view> strip_marks(std::string_view v) {
        if (v.size() < 3 || !v.starts_with('"') || !v.ends_with('"')) { return std::nullopt; }
        v.remove_prefix(1);
        v.remove_suffix(1);
        return v;
    }

    [[nodiscard]] inline auto split_at(char at, std::string_view s) {
        struct left_and_right_t {
            std::string_view left;
            std::string_view right;
        };
        using maybe_left_and_right_t = std::optional<left_and_right_t>;
        auto const sep_pos = s.find(at);
        if (sep_pos == std::string_view::npos) { return maybe_left_and_right_t{}; }
        auto const left = parsers_impl::strip(s.substr(0, sep_pos));
        if (left.size() + 1U == s.size()) { return maybe_left_and_right_t{}; }
        auto const right = lstrip(s.substr(sep_pos + 1U));
        if (right.empty()) { return maybe_left_and_right_t{}; }
        return maybe_left_and_right_t(std::in_place, left, right);
    }
}  // namespace parsers_impl

namespace json {
    namespace json_impl {
        struct key_and_value_t;
        using object_item_t = std::unique_ptr<key_and_value_t>;
        struct record_t;
    }  // namespace json_impl

    struct Null {};
    using Bool = bool;
    using String = std::string_view;
    using Number = double;
    using Object = std::vector<json_impl::object_item_t>;
    using Array = std::vector<json_impl::record_t>;
    namespace json_impl {
        struct record_t {
            using alternatives_t = std::variant<Null, Bool, String, Number, Array, Object>;
            alternatives_t json_value;
        };
    }  // namespace json_impl

    struct parsed_json_t {
        Object m_json_object;
    };

    namespace json_impl {
        struct key_and_value_t {
            std::string_view key;
            record_t value;
        };

        struct parse_result_t {
            std::optional<record_t> value;
            std::string_view remaining;
        };

        struct key_and_remaining_t {
            std::string_view key;
            std::string_view remaining;
        };

        struct parsers_traversal_output_t {
            key_and_value_t key_value;
            std::string_view remaining;
        };

        struct parse_value_output_t {
            record_t value;
            std::string_view remaining;
        };

        [[nodiscard]] std::optional<parse_value_output_t> parse_value(std::string_view fcontent);

        [[nodiscard]] inline object_item_t make_json_obj(key_and_value_t &&kv) {
            return std::make_unique<key_and_value_t>(std::move(kv));
        }

        [[nodiscard]] inline bool is_record_end(char ch) noexcept {
            return parsers_impl::is_newline(ch) || parsers_impl::char_is(ch, ',', ' ', '}', ']');
        }

        [[nodiscard]] inline std::optional<parsers_traversal_output_t> parse_key_value(std::string_view fcontent) {
            auto const key_and_to_parse = parsers_impl::split_at(':', fcontent);
            if (!key_and_to_parse) { return std::nullopt; }
            auto const [key, to_parse_after_key] = key_and_to_parse.value();
            auto maybe_key_without_marks = parsers_impl::strip_marks(key);
            if (!maybe_key_without_marks) { return std::nullopt; }
            auto parsed_value = parse_value(to_parse_after_key);
            if (!parsed_value) { return std::nullopt; }
            auto &[json_value, to_parse] = parsed_value.value();
            return parsers_traversal_output_t{.key_value = {.key = maybe_key_without_marks.value(),
                                                            .value = std::move(json_value)},
                                              .remaining = to_parse};
        }

        // TODO: could use expected<parse_result_t, fail_reason_t> and change parse_result_t to have just a record_t and not optional<record_t>
        [[nodiscard]] inline std::optional<parse_result_t> parse_keyword(std::string_view fcontent, std::string_view keyword, auto keyword_value) {  // NOLINT
            if (fcontent.size() < keyword.size() + 1 || fcontent.substr(0, keyword.size()) != keyword || !is_record_end(fcontent[keyword.size()])) {
                return parse_result_t{.value = std::nullopt, .remaining = fcontent};
            }
            return parse_result_t{.value = record_t{keyword_value},
                                  .remaining = parsers_impl::skip_newline(parsers_impl::skip_comma(fcontent.substr(keyword.size())))};
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_object(std::string_view fcontent) {
            if (fcontent[0] != '{') { return parse_result_t{.value = std::nullopt, .remaining = fcontent}; }
            fcontent = parsers_impl::lstrip(parsers_impl::skip_newline(parsers_impl::skip_brace(fcontent, '{')));
            Object json_object{};
            while (!fcontent.empty() && fcontent[0] != '}') {
                auto parsed = parse_key_value(fcontent);
                if (!parsed) { return std::nullopt; }
                auto &[key_value, remaining] = parsed.value();
                fcontent = parsers_impl::lstrip(parsers_impl::skip_newline(parsers_impl::skip_comma(remaining)));
                json_object.push_back(make_json_obj(std::move(key_value)));
            }
            if (fcontent.empty()) { return std::nullopt; }
            return parse_result_t{.value = record_t{std::move(json_object)},
                                  .remaining = parsers_impl::lstrip(parsers_impl::skip_newline(parsers_impl::skip_comma(parsers_impl::skip_brace(fcontent, '}'))))};
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_null(std::string_view fcontent) {
            return parse_keyword(fcontent, "null", Null{});
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_bool(std::string_view fcontent) {
            if (auto parse_true = parse_keyword(fcontent, "true", true);
                !parse_true || parse_true.value().value) {
                return parse_true;
            }
            return parse_keyword(fcontent, "false", false);
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_string(std::string_view fcontent) {
            if (fcontent[0] != '"') { return parse_result_t{.value = std::nullopt, .remaining = fcontent}; }
            if (auto const closing_mark = fcontent.find_first_of('"', 1); closing_mark != std::string_view::npos) {
                return parse_result_t{.value = record_t{fcontent.substr(1, closing_mark - 1U)},  // Skip marks
                                      .remaining = parsers_impl::skip_newline(parsers_impl::skip_comma(fcontent.substr(closing_mark + 1U)))};
            }
            return std::nullopt;
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_number(std::string_view fcontent) {
            if (!parsers_impl::is_number_begin(fcontent)) {
                return parse_result_t{.value = std::nullopt, .remaining = fcontent};
            }
            auto const last = std::ranges::find_if(fcontent, is_record_end);
            if (last == fcontent.end()) { return std::nullopt; }
            if (last != fcontent.begin() && *std::prev(last) == '.') { return std::nullopt; }
            auto const num = parsers_impl::sv_to_number<Number>(std::string_view{fcontent.begin(), last});
            if (!num) {
                return std::nullopt;
            }
            return parse_result_t{.value = record_t{num.value()},
                                  .remaining = parsers_impl::skip_newline(parsers_impl::skip_comma(std::string_view(last, fcontent.end())))};
        }

        [[nodiscard]] inline std::optional<parse_result_t> parse_array(std::string_view fcontent) {
            if (fcontent.empty() || !(fcontent[0] == '[')) {
                return parse_result_t{.value = std::nullopt, .remaining = fcontent};
            }
            fcontent = parsers_impl::lstrip(parsers_impl::skip_brace(fcontent, '['));
            if (fcontent.empty()) { return std::nullopt; }
            Array items{};
            while (!fcontent.empty() && fcontent[0] != ']') {
                auto item = parse_value(fcontent);
                if (!item) { return std::nullopt; }
                auto &[value, to_parse] = item.value();
                items.push_back(std::move(value));
                fcontent = parsers_impl::lstrip(to_parse);
            }
            if (fcontent.empty()) { return std::nullopt; }
            return parse_result_t{.value = record_t{std::move(items)},
                                  .remaining = parsers_impl::lstrip(parsers_impl::skip_newline(parsers_impl::skip_comma(parsers_impl::skip_brace(parsers_impl::lstrip(fcontent), ']'))))};
        }

        [[nodiscard]] inline std::optional<parse_value_output_t> parse_value(std::string_view fcontent) {
            for (auto parser : {&parse_object,
                                &parse_null,
                                &parse_bool,
                                &parse_string,
                                &parse_number,
                                &parse_array}) {
                auto parse_result = parser(fcontent);
                if (!parse_result) { return std::nullopt; }  // Invalid json
                auto &[value, to_parse_after_value] = parse_result.value();
                fcontent = to_parse_after_value;
                if (!value) { continue; }  // Current parser does not match, try the next one
                return parse_value_output_t{.value = std::move(value).value(), .remaining = fcontent};
            }
            return std::nullopt;
        }
    }  // namespace json_impl

    // TODO: this should return an expected<parsed_json_t, std::string_view>. The error being
    // the non parsed string
    [[nodiscard]] inline std::optional<parsed_json_t> parse(std::string_view fcontent) {
        if (fcontent.empty()) { return std::nullopt; }
        Object json_object{};
        auto to_parse = parsers_impl::lstrip(parsers_impl::skip_newline(parsers_impl::skip_brace(parsers_impl::lstrip(fcontent), '{')));
        while (!to_parse.empty() && to_parse[0] != '}') {
            auto parse_output = json_impl::parse_key_value(to_parse);
            if (!parse_output) { return std::nullopt; }
            auto &[parsed_object, remaining_chars] = parse_output.value();
            to_parse = parsers_impl::lstrip(remaining_chars);
            json_object.push_back(json_impl::make_json_obj(std::move(parsed_object)));
        }
        if (to_parse.empty() || to_parse[0] != '}') { return std::nullopt; }
        return parsed_json_t{std::move(json_object)};
    }
}  // namespace json
}  // namespace cpputils::parsers

#endif