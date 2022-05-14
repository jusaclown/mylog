#include "log/logger.h"
#include "log/sinks/stdout_color_sinks.h"
#include "log/mylog.h"
#include "log/sinks/rotating_file_sink.h"
#include "log/sinks/daily_file_sink.h"
#include "log/details/log_msg.h"

#include <iostream>

void stdout_logger_example();
void basic_example();
void rotating_example();
void daily_example();
void async_example();
void vector_example();
void trace_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void replace_default_logger_example();

int main()
{
    mylog::info("Welcome to mylog");
    mylog::warning("Easy padding in numbers like {:08d}", 12);
    mylog::fatal("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    mylog::info("Support for floats {:03.2f}", 1.23456);
    mylog::info("Positional args are {1} {0}..", "too", "supported");
    mylog::info("{:>8} aligned, {:<8} aligned", "right", "left");

    mylog::set_level(mylog::level::info); // Set global log level to info
    mylog::debug("This message should not be displayed!");
    mylog::set_level(mylog::level::trace); // Set specific logger's log level
    mylog::debug("This message should be displayed..");
    mylog::set_level(mylog::level::info); 
    
    try
    {
        stdout_logger_example();
        basic_example();
        rotating_example();
        daily_example();
        async_example();
        vector_example();
        trace_example();
        multi_sink_example();
        user_defined_example();
        err_handler_example();
        replace_default_logger_example();

        // Flush all *registered* loggers using a worker thread every 3 seconds.
        // note: registered loggers *must* be thread safe for this to work correctly!
        mylog::flush_every(std::chrono::seconds(3));

        // Apply some function on all registered loggers
        mylog::apply_all([&](std::shared_ptr<mylog::logger> l) { l->info("End of example."); });

        // Release all mylog resources, and drop all loggers in the registry.
        // This is optional (only mandatory if using windows + async log).
        mylog::shutdown();
    }

    // Exceptions will only be thrown upon failed logger or sink construction (not during logging).
    catch (const mylog::log_ex &ex)
    {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }
}


#include "log/sinks/stdout_color_sinks.h"
// or #include "mylog/sinks/stdout_sinks.h" if no colors needed.
void stdout_logger_example()
{
    // Create color multi threaded logger.
    auto console = mylog::stdout_color_logger_mt("console");
    // or for stderr:
    // auto console = mylog::stderr_color_mt("error-logger");
}

#include "log/sinks/basic_file_sink.h"
void basic_example()
{
    // Create basic file logger (not rotated).
    auto my_logger = mylog::basic_logger_mt("file_logger", "logs/basic-log.txt", true);
}

#include "log/sinks/rotating_file_sink.h"
void rotating_example()
{
    // Create a file rotating logger with 5mb size max and 3 rotated files.
    auto rotating_logger = mylog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
}

#include "log/sinks/daily_file_sink.h"
void daily_example()
{
    // Create a daily logger - a new file is created every day on 2:30am.
    auto daily_logger = mylog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
}

#include "log/async.h"
void async_example()
{
    // Default thread pool settings can be modified *before* creating the async logger:
    // mylog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file = mylog::basic_logger_mt<mylog::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file = mylog::create_async<mylog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");

    for (int i = 1; i < 101; ++i)
    {
        async_file->info("Async message #{}", i);
    }
}

// Log a vector of numbers

#include <fmt/ranges.h>
void vector_example()
{
    std::vector<int> vec = {1, 2, 3};
    mylog::info("Vector example: {}", vec);
}

// Compile time log levels.
void trace_example()
{
    // trace from default logger
    MYLOG_TRACE("Some trace message.. {} ,{}", 1, 3.23);
    // debug from default logger
    MYLOG_FATAL("Some debug message.. {} ,{}", 1, 3.23);

    // trace from logger object
    auto logger = mylog::get("file_logger");
    MYLOG_LOGGER_WARNING(logger, "another trace message");
}

// A logger with multiple sinks (stdout and file) - each with a different format and log level.
void multi_sink_example()
{
    auto console_sink = std::make_shared<mylog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(mylog::level::warning);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<mylog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(mylog::level::trace);

    mylog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(mylog::level::debug);
    logger.warning("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}

// User defined types logging
struct my_type
{
    int i = 0;
    explicit my_type(int i_)
        : i(i_){};
};

template<>
struct fmt::formatter<my_type> : fmt::formatter<std::string>
{
    auto format(my_type my, format_context &ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};

void user_defined_example()
{
    mylog::info("user defined type: {}", my_type(14));
}

// Custom error handler. Will be triggered on log failure.
void err_handler_example()
{
    // can be set globally or per logger(logger->set_error_handler(..))
    mylog::set_error_handler([](const std::string &msg) { printf("*** Custom log error handler: %s ***\n", msg.c_str()); });
}


void replace_default_logger_example()
{
    // store the old logger so we don't break other examples.
    auto old_logger = mylog::default_logger();

    auto new_logger = mylog::basic_logger_mt("new_default_logger", "logs/new-default-log.txt", true);
    mylog::set_default_logger(new_logger);
    mylog::set_level(mylog::level::info);
    mylog::debug("This message should not be displayed!");
    mylog::set_level(mylog::level::trace);
    mylog::debug("This message should be displayed..");

    mylog::set_default_logger(old_logger);
}
