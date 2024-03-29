#ifndef CPPUTILS_BENCHMARK_HPP
#define CPPUTILS_BENCHMARK_HPP

#include "../functional/opt_ext.hpp"
#include "../meta/traits.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

// NOLINTNEXTLINE
#define FWD(x) std::forward<decltype(x)>(x)

namespace cpputils::benchmark {

template <typename V>
concept logger_range_value =
    pair_like<V>
    && std::same_as<std::remove_cvref_t<decltype(std::declval<V>().first)>, std::string>
    && duration<std::remove_cvref_t<decltype(std::declval<V>().second)>>;

template <typename L>
concept time_logger =
    logger_range_value<std::ranges::range_reference_t<L>>
    && requires (L logger) {
           logger[std::string{}] = std::declval<typename L::mapped_type>();
       }  //
    && std::movable<L>  //
    && std::copyable<L>  //
    && duration<typename L::mapped_type>  //
    && std::ranges::input_range<L>;

template <typename C>
concept time_counter =
    std::default_initializable<C>
    && requires (C c) {
           c.start();
           c.stop();
           { c.delta() } -> duration;
       };

template <typename Counter, typename Logger>
concept time_counter_compatible_delta =
    time_logger<Logger>
    && requires (Counter c) {
           { c.delta() } -> std::convertible_to<typename Logger::mapped_type>;
       };

template <time_logger Logger>
requires std::default_initializable<Logger>
Logger &get_static_logger() {
    static Logger logger{};
    return logger;
}

using default_clock = std::chrono::steady_clock;
using default_duration = default_clock::duration;

template <typename Clock = default_clock>
class default_counter {
public:
    using clock_t = Clock;

    void start() {
        m_start = std::chrono::steady_clock::now();
    }

    void stop() {
        m_stop = std::chrono::steady_clock::now();
    }

    [[nodiscard]] duration auto delta() const {
        return m_stop - m_start;
    }

private:
    std::chrono::time_point<Clock> m_start{};
    std::chrono::time_point<Clock> m_stop{};
};

template <duration D = default_duration>
using default_logger = std::unordered_map<std::string, D>;

template <time_logger Logger = default_logger<>, time_counter TimeCounter = default_counter<>>
requires time_counter_compatible_delta<TimeCounter, Logger>
class timer {
public:
    using logger_t = Logger;
    using time_counter_t = TimeCounter;

    timer()
        : m_logger{get_static_logger<Logger>()} {}

    explicit timer(Logger &logger)
        : m_logger{logger} {}


    void start(std::string msg) {
        assert(!m_active);
        m_active = true;
        m_message = std::move(msg);
        m_time_counter.start();
    }

    void stop() {
        assert(m_active);
        m_time_counter.stop();
        m_active = false;
        m_logger.get()[m_message] = m_time_counter.delta();
    }

    Logger const &logger() const noexcept {
        return m_logger;
    }

    duration auto benchmark_this(std::string msg, auto &&f, auto &&...args)
        requires std::invocable<decltype(f), decltype(args)...>
    {
#define BENCHMARK_FUNCTION_CALL() std::invoke(FWD(f), FWD(args)...) /* NOLINT */
        start(std::move(msg));
        if constexpr (std::is_same_v<decltype(BENCHMARK_FUNCTION_CALL()), void>) {
            BENCHMARK_FUNCTION_CALL();
        } else {
            [[maybe_unused]] volatile auto const r = BENCHMARK_FUNCTION_CALL();
        }
        stop();
#undef BENCHMARK_FUNCTION_CALL
        return m_time_counter.delta();
    }

    class scoped {
    public:
        explicit scoped(Logger &logger_, std::string msg)
            : m_timeit{logger_} {
            m_timeit.start(std::move(msg));
        }

        ~scoped() {
            m_timeit.stop();
        }

        scoped(scoped const &) = delete;
        scoped(scoped &&) = delete;
        scoped &operator=(scoped const &) = delete;
        scoped &operator=(scoped &&) = delete;

    private:
        timer m_timeit;
    };

private:
    std::reference_wrapper<Logger> m_logger;
    TimeCounter m_time_counter{};
    std::string m_message{};
    bool m_active{false};
};

template <typename L, typename F>
concept report_formatter =
    requires (L logger, F f) {
        { f.type() } -> std::convertible_to<std::string_view>;
        f.parse(logger);
        { f.get() } -> std::convertible_to<std::string_view>;
    } && std::default_initializable<F> && time_logger<L>;

namespace detail {
    [[nodiscard]] inline std::optional<std::tm> cpputils_localtime(std::time_t &now_time) {
        auto const *local_time = std::localtime(&now_time);  // NOLINT
        if (local_time == nullptr) { return std::nullopt; }
        return *local_time;
    }

    [[nodiscard]] inline std::optional<std::tm> cpputils_gmtime(std::time_t &now_time) {
        auto const *local_time = std::gmtime(&now_time);  // NOLINT
        if (local_time == nullptr) { return std::nullopt; }
        return *local_time;
    }

    [[nodiscard]] inline auto today() {
        struct local_and_utc {
            std::optional<std::string> local;
            std::optional<std::string> utc;
        };
        auto const time_to_str = transform([](std::tm const &date_time) -> std::optional<std::string> {
            auto const *str = std::asctime(&date_time);  // NOLINT
            auto const *newline = static_cast<char const *>(std::memchr(str, '\n', 25));  // NOLINT See https://en.cppreference.com/w/cpp/chrono/c/asctime for the 25
            if (newline == nullptr) { return std::nullopt; }
            return std::string{str, newline};
        });

        auto now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        return local_and_utc{.local = cpputils_localtime(now_time) >> time_to_str,
                             .utc = cpputils_gmtime(now_time) >> time_to_str};
    }

    template <duration D>
    [[nodiscard]] consteval std::string_view detect_time_unit() {
        if constexpr (std::is_same_v<D, std::chrono::nanoseconds>) {
            return "nanoseconds";
        } else if constexpr (std::is_same_v<D, std::chrono::milliseconds>) {
            return "milliseconds";
        } else if constexpr (std::is_same_v<D, std::chrono::seconds>) {
            return "seconds";
        } else if constexpr (std::is_same_v<D, std::chrono::minutes>) {
            return "minutes";
        } else if constexpr (std::is_same_v<D, std::chrono::hours>) {
            return "hours";
        } else if constexpr (std::is_same_v<D, std::chrono::days>) {
            return "days";
        } else if constexpr (std::is_same_v<D, std::chrono::weeks>) {
            return "weeks";
        } else if constexpr (std::is_same_v<D, std::chrono::months>) {
            return "months";
        } else if constexpr (std::is_same_v<D, std::chrono::years>) {
            return "years";
        } else {
            return "unknown";
        }
    }

    // If logger is not sortable, copy the contents into a vector and sort it, otherwise just use logger
    template <time_logger Logger>
    [[nodiscard]] auto make_sorted(Logger logger) {
        auto const sorter = [](auto const &lhs, auto const &rhs) { return rhs.second < lhs.second; };
        if constexpr (requires { requires std::sortable<std::ranges::iterator_t<Logger>>; }) {
            std::ranges::sort(logger, sorter);
            return logger;
        } else {
            std::vector<std::pair<std::string, typename Logger::mapped_type>> copied_elements;
            if constexpr (std::ranges::sized_range<Logger>) {
                copied_elements.reserve(std::ranges::size(logger));
            }
            std::ranges::move(logger, std::back_inserter(copied_elements));
            std::ranges::sort(copied_elements, sorter);
            return copied_elements;
        }
    }

    template <std::ranges::view V>
    requires std::ranges::input_range<V> && std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<V>>, std::string>
    [[nodiscard]] std::string join_with(std::string_view sep, V rng) {
        auto const rng_size = std::ranges::size(rng);
        auto iteration = decltype(rng_size){0};
        auto report = std::string{};
        for (auto const &record : rng) {
            ++iteration;
            if (iteration < rng_size) {
                report += record + std::string{sep};
            } else {
                report += record;
            }
        }
        return report;
    };

    template <typename DerivedFormatter>
    struct base_formatter {
        template <time_logger Logger>
        void parse(Logger logger) {
            static constexpr auto time_unit = detect_time_unit<typename Logger::mapped_type>();
            auto &self = underlying();
            self.start(time_unit);
            std::ranges::input_range auto const sorted = detail::make_sorted(std::move(logger));
            auto const time_values = sorted | std::views::transform([this](auto const &kv) { return underlying().process_data(kv); });
            auto const records = join_with(self.separator(), time_values);
            self.m_content += records;
            self.finish();
        }

        [[nodiscard]] std::string_view get() const { return underlying().m_content; }

    private:
        DerivedFormatter &underlying() {
            return static_cast<DerivedFormatter &>(*this);
        }
        [[nodiscard]] DerivedFormatter const &underlying() const {
            return static_cast<DerivedFormatter const &>(*this);
        }
    };
}  // namespace detail

namespace formatters {
    struct csv : public detail::base_formatter<csv> {
        friend detail::base_formatter<csv>;

        [[nodiscard]] static std::string_view type() noexcept { return "csv"; }

    private:
        std::string m_content{};
        std::string m_unit_of_measure{};

        [[nodiscard]] static std::string_view separator() noexcept { return "\n"; }

        void start(std::string_view unit_of_measure) {
            m_unit_of_measure = std::string{unit_of_measure};
            m_content = "description,elapsed_time,unit_of_measure\n";
        }
        [[nodiscard]] std::string process_data(pair_like auto const kv) const {
            auto const elapsed_time = std::to_string(kv.second.count());
            return kv.first + "," + elapsed_time + "," + m_unit_of_measure;
        }
        void finish() { m_content += '\n'; }
    };

    struct json : detail::base_formatter<json> {
        friend detail::base_formatter<json>;
        [[nodiscard]] static std::string_view type() noexcept { return "json"; }

    private:
        std::string m_content{};

        [[nodiscard]] static std::string_view separator() noexcept { return ",\n"; }

        void start(std::string_view unit_of_measure) {
            auto const times = []() {
                auto const [local_time, utc_time] = detail::today();
                std::string times_{};
                if (local_time) { times_ += "\n    local_time: \"" + local_time.value() + "\","; }
                if (utc_time) { times_ += "\n    utc_time: \"" + utc_time.value() + "\","; }
                return times_;
            }();
            m_content = std::string{"{"} + times + "\n    time_unit: \"" + unit_of_measure.data() + "\",\n";
        }
        [[nodiscard]] static std::string process_data(pair_like auto const kv) {
            auto const elapsed_time = std::to_string(kv.second.count());
            return std::string{"    "} + kv.first + ": " + elapsed_time;
        }
        void finish() { m_content += "\n}\n"; }
    };

    struct yaml : detail::base_formatter<yaml> {
        friend detail::base_formatter<yaml>;
        [[nodiscard]] static std::string_view type() noexcept { return "yaml"; }

    private:
        std::string m_content{};

        [[nodiscard]] static std::string_view separator() noexcept { return "\n"; }

        void start(std::string_view unit_of_measure) {
            auto const times = []() {
                auto const [local_time, utc_time] = detail::today();
                std::string times_{};
                if (local_time) { times_ += "local_time: \"" + local_time.value() + "\"\n"; }
                if (utc_time) { times_ += "utc_time: \"" + utc_time.value() + "\"\n"; }
                return times_;
            }();
            m_content = times + "time_unit: \"" + unit_of_measure.data() + "\"\n";
        }
        [[nodiscard]] static std::string process_data(pair_like auto const kv) {
            auto const elapsed_time = std::to_string(kv.second.count());
            return kv.first + ": " + elapsed_time;
        }
        void finish() { m_content += "\n"; }
    };

    struct markdown : detail::base_formatter<markdown> {
        friend detail::base_formatter<markdown>;
        [[nodiscard]] static std::string_view type() noexcept { return "markdown"; }

    private:
        std::string m_content{};

        [[nodiscard]] static std::string_view separator() noexcept { return "\n"; }

        void start(std::string_view unit_of_measure) {
            m_content = "# Benchmark Report\n\n";
            auto const times = []() -> std::string {
                auto const [local_time, utc_time] = detail::today();
                if (!local_time && !utc_time) { return ""; }
                std::string times_ = "## Date time\n\n";
                if (local_time) { times_ += "- Local time: " + local_time.value() + "\n"; }
                if (utc_time) { times_ += "- UTC time: " + utc_time.value() + "\n"; }
                times_ += "\n";
                return times_;
            }();
            m_content += times + "## Unit of measure\n\n-Unit: " + unit_of_measure.data() + "\n\n## Data\n\n";
        }
        [[nodiscard]] static std::string process_data(pair_like auto const kv) {
            auto const elapsed_time = std::to_string(kv.second.count());
            return std::string{"- *"} + kv.first + "*: " + elapsed_time;
        }
        void finish() { m_content += "\n"; }
    };

    struct html : detail::base_formatter<html> {
        friend detail::base_formatter<html>;
        [[nodiscard]] static std::string_view type() noexcept { return "html"; }

    private:
        std::string m_content{};

        void start(std::string_view unit_of_measure) {
            m_content =
                "<!doctype html>\n"
                "  <html>\n"
                "    <head>\n"
                "      <meta charset=\"utf-8\">\n"
                "      <title>Benchmark Report</title>\n"
                "      <meta name=\"Report of registerd timestamps\">\n"
                "    </head>\n"
                "    <body>\n"
                "    <h1>Benchmark Report</h1>\n\n";
            auto const times = []() -> std::string {
                auto const [local_time, utc_time] = detail::today();
                if (!local_time && !utc_time) { return ""; }
                std::string times_ =
                    "      <h2>Date and time</h2>\n\n"
                    "      <ul>\n";

                if (local_time) { times_ += "        <li>Local: " + local_time.value() + "</li>\n"; }
                if (utc_time) { times_ += "        <li>Local: " + utc_time.value() + "</li>\n"; }
                times_ += "      </ul>\n\n";
                return times_;
            }();
            m_content += times + "      <h2>Unit of measure</h2>\n\n      " + unit_of_measure.data() + "\n\n";
            m_content += "      <h2>Data</h2>\n\n      <ul>\n";
        }

        [[nodiscard]] static std::string process_data(pair_like auto const kv) {
            auto const elapsed_time = std::to_string(kv.second.count());
            return std::string{"        <li>"} + kv.first + ": " + elapsed_time + "</li>";
        }

        [[nodiscard]] static std::string_view separator() noexcept { return "\n"; }

        void finish() {
            m_content += "\n      </ul>\n\n    </body>\n  </html>\n\n";
        }
    };
}  // namespace formatters

template <typename... AdditionalFormatters>
class reporter {
public:
    template <typename Formatter, time_logger Logger>
    void format_as(Logger const &logger) requires report_formatter<Logger, Formatter>
    {
        m_formatter.template emplace<Formatter>();
        std::visit([&logger](auto &f) { f.parse(logger); }, m_formatter);
    }

    [[nodiscard]] std::string_view get_report() const {
        return std::visit([](auto const &f) { return f.get(); }, m_formatter);
    }

    [[nodiscard]] std::string_view get_format() const {
        return std::visit([](auto const &f) { return f.type(); }, m_formatter);
    }

    template <typename Formatter, time_logger Logger>
    requires report_formatter<Logger, Formatter>
    [[nodiscard]] static std::string report(Logger const &logger) {
        reporter log_reporter{};
        log_reporter.format_as<Formatter>(logger);
        return std::string{log_reporter.get_report()};
    }

private:
    std::variant<formatters::json,
                 formatters::csv,
                 formatters::yaml,
                 formatters::markdown,
                 formatters::html,
                 AdditionalFormatters...>
        m_formatter{};
};
}  // namespace cpputils::benchmark

#undef FWD

#endif
