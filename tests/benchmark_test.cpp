#include "cpputils/misc/benchmark.hpp"
#include <catch2/catch_all.hpp>
#include <chrono>
#include <iostream>
#include <thread>


using namespace cpputils::benchmark;

using namespace std::literals;

TEST_CASE("benchmark-should-compile", "[check-compilation]") {
    // TODO:
    // SUCCEED();
    auto logger = timer<>::logger_t{};
    {
        timer<>::scoped const t{logger, "section_1"};
        std::this_thread::sleep_for(350ms);
    }
    {
        timer<>::scoped const t{logger, "section_2"};
        std::this_thread::sleep_for(100ms);
    }
    {
        timer<>::scoped const t{logger, "section_3"};
        std::this_thread::sleep_for(503ms);
    }
    {
        timer<>::scoped const t{logger, "section_4"};
        std::this_thread::sleep_for(236005000ns);
    }
    auto const time_report = reporter<>::report<formatters::csv>(logger);

    std::cout << time_report;
}