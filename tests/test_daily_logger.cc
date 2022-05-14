/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/mylog/master/LICENSE
 */
#include "includes.h"


using filename_memory_buf_t = fmt::basic_memory_buffer<mylog::filename_t::value_type, 250>;


TEST_CASE("daily_logger with dateonly calculator", "[daily_logger]")
{
    using sink_type = mylog::sinks::daily_file_sink<std::mutex, mylog::sinks::daily_filename_calculator>;

    prepare_logdir();

    // calculate filename (time based)
    mylog::filename_t basename = MYLOG_FILENAME_T("test_logs/daily_dateonly");
    std::tm tm = mylog::details::os::localtime();
    filename_memory_buf_t w;
    fmt::format_to(
        std::back_inserter(w), MYLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = mylog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }
    logger->flush();

    auto filename = fmt::to_string(w);
    require_message_count(filename, 10);
}

struct custom_daily_file_name_calculator
{
    static mylog::filename_t calc_filename(const mylog::filename_t &basename, const tm &now_tm)
    {
        filename_memory_buf_t w;
        fmt::format_to(std::back_inserter(w), MYLOG_FILENAME_T("{}{:04d}{:02d}{:02d}"), basename, now_tm.tm_year + 1900,
            now_tm.tm_mon + 1, now_tm.tm_mday);

        return fmt::to_string(w);
    }
};

TEST_CASE("daily_logger with custom calculator", "[daily_logger]")
{
    using sink_type = mylog::sinks::daily_file_sink<std::mutex, custom_daily_file_name_calculator>;

    prepare_logdir();

    // calculate filename (time based)
    mylog::filename_t basename = MYLOG_FILENAME_T("test_logs/daily_dateonly");
    std::tm tm = mylog::details::os::localtime();
    filename_memory_buf_t w;
    fmt::format_to(
        std::back_inserter(w), MYLOG_FILENAME_T("{}{:04d}{:02d}{:02d}"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    auto logger = mylog::create<sink_type>("logger", basename, 0, 0);
    for (int i = 0; i < 10; ++i)
    {
        logger->info("Test message {}", i);
    }

    logger->flush();


    auto filename = fmt::to_string(w);
    require_message_count(filename, 10);
}


// regex supported only from gcc 4.9 and above
#if defined(_MSC_VER) || !(__GNUC__ <= 4 && __GNUC_MINOR__ < 9)

#    include <regex>

TEST_CASE("daily_file_sink::daily_filename_calculator", "[daily_file_sink]]")
{
    // daily_YYYY-MM-DD_hh-mm.txt
    auto filename =
        mylog::sinks::daily_filename_calculator::calc_filename(MYLOG_FILENAME_T("daily.txt"), mylog::details::os::localtime());
    // date regex based on https://www.regular-expressions.info/dates.html
    std::basic_regex<mylog::filename_t::value_type> re(
        MYLOG_FILENAME_T(R"(^daily_(19|20)\d\d-(0[1-9]|1[012])-(0[1-9]|[12][0-9]|3[01])\.txt$)"));
    std::match_results<mylog::filename_t::const_iterator> match;
    REQUIRE(std::regex_match(filename, match, re));
}
#endif

TEST_CASE("daily_file_sink::daily_filename_format_calculator", "[daily_file_sink]]")
{
    std::tm tm = mylog::details::os::localtime();
    // example-YYYY-MM-DD.log
    auto filename = mylog::sinks::daily_filename_format_calculator::calc_filename(MYLOG_FILENAME_T("example-%Y-%m-%d.log"), tm);

    REQUIRE(filename ==
            fmt::format(MYLOG_FILENAME_T("example-{:04d}-{:02d}-{:02d}.log"), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday));
}

/* Test removal of old files */
static mylog::details::log_msg create_msg(std::chrono::seconds offset)
{
    using mylog::log_clock;
    mylog::details::log_msg msg{"test", mylog::level::info, "Hello Message"};
    msg.time = log_clock::now() + offset;
    return msg;
}

static void test_rotate(int days_to_run, uint16_t max_days, uint16_t expected_n_files)
{
    using mylog::log_clock;
    using mylog::details::log_msg;
    using mylog::sinks::daily_file_sink_st;

    prepare_logdir();

    mylog::filename_t basename = MYLOG_FILENAME_T("test_logs/daily_rotate.txt");
    daily_file_sink_st sink{basename, 2, 30, true, max_days};

    // simulate messages with 24 intervals

    for (int i = 0; i < days_to_run; i++)
    {
        auto offset = std::chrono::seconds{24 * 3600 * i};
        sink.log(create_msg(offset));
    }

    REQUIRE(count_files("test_logs") == static_cast<size_t>(expected_n_files));
}

TEST_CASE("daily_logger rotate", "[daily_file_sink]")
{
    int days_to_run = 1;
    test_rotate(days_to_run, 0, 1);
    test_rotate(days_to_run, 1, 1);
    test_rotate(days_to_run, 3, 1);
    test_rotate(days_to_run, 10, 1);

    days_to_run = 10;
    test_rotate(days_to_run, 0, 10);
    test_rotate(days_to_run, 1, 1);
    test_rotate(days_to_run, 3, 3);
    test_rotate(days_to_run, 9, 9);
    test_rotate(days_to_run, 10, 10);
    test_rotate(days_to_run, 11, 10);
    test_rotate(days_to_run, 20, 10);
}
