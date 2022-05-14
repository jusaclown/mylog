//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : mylog benchmarks
//
#include "log/mylog.h"
#include "log/sinks/basic_file_sink.h"
#include "log/sinks/daily_file_sink.h"
#include "log/sinks/rotating_file_sink.h"

#include <fmt/format.h>
#include <fmt/core.h>
#include <atomic>
#include <cstdlib> // EXIT_FAILURE
#include <memory>
#include <string>
#include <thread>


void bench(int howmany, std::shared_ptr<mylog::logger> log);
void bench_mt(int howmany, std::shared_ptr<mylog::logger> log, size_t thread_count);


static const size_t file_size = 30 * 1024 * 1024;
static const size_t rotating_files = 5;
static const int max_threads = 1000;

void bench_threaded_logging(size_t threads, int iters)
{
    mylog::info("**************************************************************");
    mylog::info(fmt::format(std::locale("en_US.UTF-8"), "Multi threaded: {:L} threads, {:L} messages", threads, iters));
    mylog::info("**************************************************************");

    auto basic_mt = mylog::basic_logger_mt("basic_mt", "logs/basic_mt.log", true);
    bench_mt(iters, std::move(basic_mt), threads);

    mylog::info("");
    auto rotating_mt = mylog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
    bench_mt(iters, std::move(rotating_mt), threads);

    mylog::info("");
    auto daily_mt = mylog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
    bench_mt(iters, std::move(daily_mt), threads);

    mylog::info("");
    auto empty_logger = std::make_shared<mylog::logger>("level-off");
    empty_logger->set_level(mylog::level::off);
    bench(iters, empty_logger);
}

void bench_single_threaded(int iters)
{
    mylog::info("**************************************************************");
    mylog::info(fmt::format(std::locale("en_US.UTF-8"), "Single threaded: {} messages", iters));
    mylog::info("**************************************************************");

    auto basic_st = mylog::basic_logger_st("basic_st", "logs/basic_st.log", true);
    bench(iters, std::move(basic_st));

    mylog::info("");
    auto rotating_st = mylog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
    bench(iters, std::move(rotating_st));

    mylog::info("");
    auto daily_st = mylog::daily_logger_st("daily_st", "logs/daily_st.log");
    bench(iters, std::move(daily_st));

    mylog::info("");
    auto empty_logger = std::make_shared<mylog::logger>("level-off");
    empty_logger->set_level(mylog::level::off);
    bench(iters, empty_logger);
}

int main(int argc, char *argv[])
{
    mylog::set_automatic_registration(false);
    mylog::default_logger()->set_pattern("[%^%l%$] %v");
    int iters = 500000;
    size_t threads = 4;
    try
    {

        if (argc > 1)
        {
            iters = std::stoi(argv[1]);
        }
        if (argc > 2)
        {
            threads = std::stoul(argv[2]);
        }

        if (threads > max_threads)
        {
            throw std::runtime_error(fmt::format("Number of threads exceeds maximum({})", max_threads));
        }

        bench_single_threaded(iters);
        bench_threaded_logging(1, iters);
        bench_threaded_logging(threads, iters);
    }
    catch (std::exception &ex)
    {
        mylog::error(ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void bench(int howmany, std::shared_ptr<mylog::logger> log)
{
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        log->info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    mylog::info(fmt::format(
        std::locale("en_US.UTF-8"), "{:<30} Elapsed: {:0.2f} secs {:>16L}/sec", log->name(), delta_d, int(howmany / delta_d)));
    mylog::drop(log->name());
}

void bench_mt(int howmany, std::shared_ptr<mylog::logger> log, size_t thread_count)
{
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    auto start = high_resolution_clock::now();
    for (size_t t = 0; t < thread_count; ++t)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < howmany / static_cast<int>(thread_count); j++)
            {
                log->info("Hello logger: msg number {}", j);
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    };

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    mylog::info(fmt::format(
        std::locale("en_US.UTF-8"), "{:<30} Elapsed: {:0.2f} secs {:>16L}/sec", log->name(), delta_d, int(howmany / delta_d)));
    mylog::drop(log->name());
}