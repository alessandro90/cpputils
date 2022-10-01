#ifndef CPPUTILS_BENCHMARK_HPP
#define CPPUTILS_BENCHMARK_HPP

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
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>


namespace cpputils::benchmark {
template <typename>
struct is_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <typename D>
concept duration = is_duration<D>::value;

template <typename L>
concept time_logger =
    requires (L logger) {
        logger[std::string{}] = std::declval<typename L::mapped_type>();
    } && duration<typename L::mapped_type>;

template <typename C>
concept time_counter =
    std::default_initializable<C>
    && requires (C c) {
           c.start();
           c.stop();
           { c.delta() } -> duration;
       };

template <time_logger Logger>
requires std::default_initializable<Logger>
Logger &make_static_logger() {
    static Logger logger{};
    return logger;
}

template <time_logger Logger, time_counter TimeCounter>
class timer {
public:
    using logger_t = Logger;
    using time_counter_t = TimeCounter;

    timer()
        : m_logger{make_static_logger<Logger>()} {}

    explicit timer(Logger &logger)
        : m_logger{logger} {}


    void start(std::string msg) {
        assert(!m_active);
        m_active = true;
        m_message = std::move(msg);
        m_time_counter.start();
    }

    void stop() {
        m_active = false;
        m_time_counter.stop();
        m_logger.get()[m_message] = m_time_counter.delta();
    }

    Logger const &logger() const noexcept {
        return m_logger;
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

using default_clock = std::chrono::steady_clock;
using default_duration = default_clock::duration;

class default_counter {
public:
    void start() {
        m_start = std::chrono::steady_clock::now();
    }

    void stop() {
        m_stop = std::chrono::steady_clock::now();
    }

    [[nodiscard]] auto delta() const {
        return m_stop - m_start;
    }

private:
    std::chrono::time_point<default_clock> m_start{};
    std::chrono::time_point<default_clock> m_stop{};
};

template <duration D>
using default_container = std::unordered_map<std::string, D>;

template <duration D = default_duration>
using default_logger = default_container<D>;

template <typename T>
concept pair_like =
    requires (T t) {
        t.first;
        t.second;
    };

template <typename L, typename F>
concept report_formatter =
    requires (L logger, F f) {
        { f.type() } -> std::convertible_to<std::string_view>;
        f.parse(logger);
        { f.get() } -> std::convertible_to<std::string_view>;
    } && std::default_initializable<F> && time_logger<L>;

namespace detail {
    struct local_and_utc {
        std::optional<std::string> local;
        std::optional<std::string> utc;
    };

    [[nodiscard]] local_and_utc today() {
        auto now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm local_time_buf{};
        std::tm utc_time_buf{};
        auto *local_time = localtime_r(&now_time, &local_time_buf);  // NOLINT
        auto *utc_time = gmtime_r(&now_time, &utc_time_buf);  // NOLINT
        auto const time_to_maybe_str = [](std::tm *validation_check, std::tm const &date_time) -> std::optional<std::string> {
            if (!validation_check) {
                return std::nullopt;
            } else {
                auto const *str = std::asctime(&date_time);  // NOLNT
                auto const *newline = static_cast<char const *>(std::memchr(str, '\n', 25));  // NOLINT See https://en.cppreference.com/w/cpp/chrono/c/asctime for the 25
                if (newline == nullptr) { return std::nullopt; }
                return std::string{str, newline};
            }
        };
        return local_and_utc{
            .local = time_to_maybe_str(local_time, local_time_buf),
            .utc = time_to_maybe_str(utc_time, utc_time_buf)};
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
    decltype(auto) make_sorted(Logger const &logger) {
        auto const sorter = [](auto lhs, auto rhs) { return rhs.second < lhs.second; };
        if constexpr (requires { requires std::sortable<std::ranges::iterator_t<Logger>>; }) {
            std::ranges::sort(logger, sorter);
            return (logger);  // return a reference
        } else {
            std::vector<std::pair<std::string, typename Logger::mapped_type>> copied_elements;
            copied_elements.reserve(std::ranges::size(logger));
            std::ranges::copy(logger, std::back_inserter(copied_elements));
            std::ranges::sort(copied_elements, sorter);
            return copied_elements;
        }
    }

    std::string join_with(std::string_view sep, std::ranges::view auto rng) {
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
        void parse(Logger const &logger) {
            static constexpr auto time_unit = detect_time_unit<typename Logger::mapped_type>();
            auto &self = underlying();
            self.start(time_unit);
            decltype(auto) sorted = detail::make_sorted(logger);
            auto const time_values = sorted | std::views::transform([](auto const &kv) { return DerivedFormatter::process_data(kv); });
            auto const records = join_with(self.separator(), std::views::all(time_values));
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

        [[nodiscard]] static std::string_view separator() noexcept { return ","; }

        void start(std::string_view unit_of_measure) {
            auto const times = []() {
                auto const [local_time, utc_time] = detail::today();
                std::string times{};
                if (local_time) { times += "local_time," + local_time.value() + ","; }
                if (utc_time) { times += "utc_time," + utc_time.value() + ","; }
                return times;
            }();
            m_content = times + "time_unit" + "," + unit_of_measure.data() + ",";
        }
        [[nodiscard]] static std::string process_data(pair_like auto const kv) {
            auto const elapsed_time = std::to_string(kv.second.count());
            return kv.first + "," + elapsed_time;
        }
        void finish() { m_content += "\n"; }
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
                std::string times{};
                if (local_time) { times += "\n    local_time: " + local_time.value() + ","; }
                if (utc_time) { times += "\n    utc_time: " + utc_time.value() + ","; }
                return times;
            }();
            m_content = std::string{"{"} + times + "\n    time_unit: " + unit_of_measure.data() + ",\n";
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
                std::string times{};
                if (local_time) { times += "local_time: " + local_time.value() + "\n"; }
                if (utc_time) { times += "utc_time: " + utc_time.value() + "\n"; }
                return times;
            }();
            m_content = times + "time_unit: " + unit_of_measure.data() + "\n";
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
                std::string times = "## Date time\n\n";
                if (local_time) { times += "- Local time: " + local_time.value() + "\n"; }
                if (utc_time) { times += "- UTC time: " + utc_time.value() + "\n"; }
                times += "\n";
                return times;
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
                std::string times =
                    "      <h2>Date and time</h2>\n\n"
                    "      <ul>\n";

                if (local_time) { times += "        <li>Local: " + local_time.value() + "</li>\n"; }
                if (utc_time) { times += "        <li>Local: " + utc_time.value() + "</li>\n"; }
                times += "      </ul>\n\n";
                return times;
            }();
            m_content += times + "      <h2>Unit of measure</h2>\n\n" + "      " + unit_of_measure.data() + "\n\n";
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
        m_formatter.emplace<Formatter>();
        std::visit([&logger](auto &f) { f.parse(logger); }, m_formatter);
    }

    [[nodiscard]] std::string_view get_report() const {
        return std::visit([](auto const &f) { return f.get(); }, m_formatter);
        ;
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
#endif
