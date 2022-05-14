#include "includes.h"

using mylog::memory_buf_t;

TEST_CASE("clone-default-formatter", "[pattern_formatter]")
{
    auto formatter_1 = std::make_shared<mylog::pattern_formatter>();
    auto formatter_2 = formatter_1->clone();
    std::string logger_name = "test";
    mylog::details::log_msg msg(logger_name, mylog::level::info, "some message");

    memory_buf_t formatted_1;
    memory_buf_t formatted_2;
    formatter_1->format(msg, formatted_1);
    formatter_2->format(msg, formatted_2);

    REQUIRE(fmt::to_string(formatted_1) == fmt::to_string(formatted_2));
}

TEST_CASE("clone-default-formatter2", "[pattern_formatter]")
{
    auto formatter_1 = std::make_shared<mylog::pattern_formatter>("%+");
    auto formatter_2 = formatter_1->clone();
    std::string logger_name = "test";
    mylog::details::log_msg msg(logger_name, mylog::level::info, "some message");

    memory_buf_t formatted_1;
    memory_buf_t formatted_2;
    formatter_1->format(msg, formatted_1);
    formatter_2->format(msg, formatted_2);
    
    REQUIRE(fmt::to_string(formatted_1) == fmt::to_string(formatted_2));
}