#include "log/mylog.h"
#include "log/async.h"
#include "log/sinks/basic_file_sink.h"

#include <fmt/format.h>
#include <fmt/core.h>
#include <atomic>
#include <cstdlib> // EXIT_FAILURE
#include <memory>
#include <string>
#include <thread>
#include <iostream>


using namespace std;
using namespace std::chrono;
using namespace mylog;
using namespace mylog::sinks;


void bench_mt(int howmany, std::shared_ptr<mylog::logger> logger, int thread_count);

int count_lines(const char* filename)
{
    int counter = 0;
    auto* infile = fopen(filename, "r");
    int ch;
    while (EOF != (ch = getc(infile)))
    {
        if ('n' == ch)
            counter++;
    }
    fclose(infile);
    return counter;
}

void verify_file(const char *filename, int expected_count)
{
    mylog::info("Verifying {} to contain {} line..", filename, expected_count);
    auto count = count_lines(filename);
    if (count != expected_count)
    {
        mylog::error("Test failed. {} has {} lines instead of {}", filename, count, expected_count);
        exit(1);
    }
    mylog::info("Line count OK ({})\n", count);
}

int main(int argc, char *argv[])
{

    int howmany = 1000000;
    int queue_size = std::min(howmany + 2, 8192);
    int threads = 10;
    int iters = 3;
    int thread_pool_thread = 1;

    try
    {
        mylog::set_pattern("[%^%l%$] %v");

        if (argc > 1)
            howmany = atoi(argv[1]);
        if (argc > 2)
            threads = atoi(argv[2]);
        if (argc > 3)
        {
            queue_size = atoi(argv[3]);
            if (queue_size > 500000)
            {
                mylog::error("Max queue size allowed: 500,000");
                exit(1);
            }
        }

        if (argc > 4)
            iters = atoi(argv[4]);

        if (argc > 5)
            thread_pool_thread = atoi(argv[4]);

        auto slot_size = sizeof(mylog::details::async_msg);
        mylog::info("-------------------------------------------------");
        mylog::info("Messages     : {:L}", howmany);
        mylog::info("Threads      : {:L}", threads);
        mylog::info("Queue        : {:L} slots", queue_size);
        mylog::info("Queue memory : {:L} x {:L} = {:L} KB ", queue_size, slot_size, (queue_size * slot_size) / 1024);
        mylog::info("Total iters  : {:L}", iters);
        mylog::info("-------------------------------------------------");

        const char *filename = "logs/basic_async.log";
        mylog::info("");
        mylog::info("*********************************");
        mylog::info("Queue Overflow Policy: block");
        mylog::info("*********************************");
        for (int i = 0; i < iters; i++)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, thread_pool_thread);
            auto file_sink = std::make_shared<mylog::sinks::basic_file_sink_mt>(filename, true);
            auto logger = std::make_shared<async_logger>("async_logger", std::move(file_sink), std::move(tp), async_overflow_policy::block);
            bench_mt(howmany, std::move(logger), threads);
            // verify_file(filename, howmany);
        }

        mylog::info("");
        mylog::info("*********************************");
        mylog::info("Queue Overflow Policy: overrun");
        mylog::info("*********************************");
        // do same test but discard oldest if queue is full instead of blocking
        filename = "logs/basic_async-overrun.log";
        for (int i = 0; i < iters; i++)
        {
            auto tp = std::make_shared<details::thread_pool>(queue_size, thread_pool_thread);
            auto file_sink = std::make_shared<mylog::sinks::basic_file_sink_mt>(filename, true);
            auto logger =
                std::make_shared<async_logger>("async_logger", std::move(file_sink), std::move(tp), async_overflow_policy::overrun_oldest);
            bench_mt(howmany, std::move(logger), threads);
        }
        mylog::shutdown();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        perror("Last error");
        return 1;
    }
    return 0;
}

void thread_fun(std::shared_ptr<mylog::logger> logger, int howmany)
{
    for (int i = 0; i < howmany; i++)
    {
        logger->info("Hello logger: msg number {}", i);
    }
}

void bench_mt(int howmany, std::shared_ptr<mylog::logger> logger, int thread_count)
{
    using std::chrono::steady_clock;
    vector<std::thread> threads;
    auto start = steady_clock::now();

    int msgs_per_thread = howmany / thread_count;
    int msgs_per_thread_mod = howmany % thread_count;
    for (int t = 0; t < thread_count; ++t)
    {
        if (t == 0 && msgs_per_thread_mod)
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread + msgs_per_thread_mod));
        else
            threads.push_back(std::thread(thread_fun, logger, msgs_per_thread));
    }

    for (auto& t : threads)
    {
        t.join();
    }
    
    auto delta = steady_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    mylog::info("Elapsed: {} secs\t {:L}/sec", delta_d, int(howmany / delta_d));
}