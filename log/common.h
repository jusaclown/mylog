#pragma once

#include <atomic>
#include <fmt/core.h>
#include <fmt/format.h>
#include <chrono>
#include <string>
#include <exception>
#include <initializer_list>
#include <functional>

namespace mylog {

class async_logger;

namespace sinks {
    class sink;
}

using string_view_t = fmt::basic_string_view<char>;
using memory_buf_t = fmt::basic_memory_buffer<char, 250>;

using log_clock = std::chrono::system_clock;
using filename_t = std::string;
using level_t = std::atomic<int>;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using err_handler = std::function<void(const std::string& err_msg)>;
using async_logger_ptr = std::shared_ptr<async_logger>;

struct source_loc
{
    constexpr source_loc() = default;
    constexpr source_loc(const char* filename_, std::size_t line_, const char* funname_)
        : filename(filename_)
        , line(line_)
        , funname(funname_)
    {}

    constexpr bool empty() const noexcept
    {
        return line == 0;
    }

    const char* filename{ nullptr };
    std::size_t line{ 0 };
    const char* funname{ nullptr };
};


// 异常相关
class log_ex : public std::exception
{
public:
    explicit log_ex(std::string msg)
        : msg_(std::move(msg))
    {}

    log_ex(const std::string& msg, int last_errno)
    {
        memory_buf_t buf;
        fmt::format_system_error(buf, last_errno, msg.c_str());
        msg_ = fmt::to_string(buf);
    }

    const char* what() const noexcept
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

// noreturn不是用来告诉编译器该函数不返回任何值，它告诉编译器控制流不会返回给调用者。
// throw_mylog_ex("Failed opening file", errno);
// 1. 生成string -> 只要复制一次
[[noreturn]] inline void throw_mylog_ex(const std::string& msg, int last_errno)
{
    throw(log_ex(msg, last_errno));
}

// 只要复制一次，如果用const std::string& msg，则生成msg复制一次，赋值给msg_又复制一次
[[noreturn]] inline void throw_mylog_ex(std::string msg)
{
    throw(log_ex(std::move(msg)));
}

} // namespace mylog
