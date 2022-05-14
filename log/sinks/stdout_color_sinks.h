#pragma once

#include "log/sinks/sink.h"
#include "log/details/log_msg.h"
#include "log/common.h"
#include "log/level.h"
#include "log/details/console_global.h"
#include "log/synchronous_factory.h"
#include "log/pattern_formatter.h"

#include <array>


namespace mylog {
namespace sinks {

template<typename ConsoleMutex>
class stdout_color_sink_base : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    
    explicit stdout_color_sink_base(std::FILE* file)
        : file_(file)
        , mutex_(ConsoleMutex::mutex())
        , formatter_(new pattern_formatter())
    {
        colors_[level::trace] = to_string_(white);
        colors_[level::debug] = to_string_(cyan);
        colors_[level::info] = to_string_(green);
        colors_[level::warning] = to_string_(yellow_bold);
        colors_[level::error] = to_string_(red_bold);
        colors_[level::fatal] = to_string_(bold_on_red);
        colors_[level::off] = to_string_(reset);
    }
    ~stdout_color_sink_base() = default;
    
    stdout_color_sink_base(const stdout_color_sink_base&) = delete;
    stdout_color_sink_base& operator=(const stdout_color_sink_base&) = delete;
    
    stdout_color_sink_base(stdout_color_sink_base&&) = delete;
    stdout_color_sink_base& operator=(stdout_color_sink_base&&) = delete;

    void set_color(level::level_enum lvl, const string_view_t& color)
    {
        std::lock_guard<mutex_t> lock(mutex_);
        colors_[static_cast<size_t>(lvl)] = to_string_(color);
    }

    void log(const details::log_msg& msg) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        memory_buf_t buf;
        formatter_->format(msg, buf);
        
        if (msg.color_range_end > msg.color_range_start)
        {
            // 1. 打印颜色前的部分
            print_range_(0, msg.color_range_start, buf);
            // 2. 打印颜色部分
            print_ccode_(colors_[static_cast<size_t>(msg.level)]);
            print_range_(msg.color_range_start, msg.color_range_end, buf);
            print_ccode_(reset);
            // 3. 打印颜色后面的部分
            print_range_(msg.color_range_end, buf.size(), buf);
        }
        else
        {
            print_range_(0, buf.size(), buf);
        }
        fflush(file_);  // 每条日志都刷新缓冲区
    }
    
    void flush() override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        std::fflush(file_);
    }
    
    void set_pattern(const std::string& pattern) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::make_unique<pattern_formatter>(pattern);
    }
    
    void set_formatter(std::unique_ptr<mylog::formatter> sink_formatter) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::move(sink_formatter);
    }
    
    // Formatting codes
    const string_view_t reset = "\033[m";
    const string_view_t bold = "\033[1m";
    const string_view_t dark = "\033[2m";
    const string_view_t underline = "\033[4m";
    const string_view_t blink = "\033[5m";
    const string_view_t reverse = "\033[7m";
    const string_view_t concealed = "\033[8m";
    const string_view_t clear_line = "\033[K";

    // Foreground colors
    const string_view_t black = "\033[30m";
    const string_view_t red = "\033[31m";
    const string_view_t green = "\033[32m";
    const string_view_t yellow = "\033[33m";
    const string_view_t blue = "\033[34m";
    const string_view_t magenta = "\033[35m";
    const string_view_t cyan = "\033[36m";
    const string_view_t white = "\033[37m";

    /// Background colors
    const string_view_t on_black = "\033[40m";
    const string_view_t on_red = "\033[41m";
    const string_view_t on_green = "\033[42m";
    const string_view_t on_yellow = "\033[43m";
    const string_view_t on_blue = "\033[44m";
    const string_view_t on_magenta = "\033[45m";
    const string_view_t on_cyan = "\033[46m";
    const string_view_t on_white = "\033[47m";

    /// Bold colors
    const string_view_t yellow_bold = "\033[33m\033[1m";
    const string_view_t red_bold = "\033[31m\033[1m";
    const string_view_t bold_on_red = "\033[1m\033[41m";

private:
    void print_ccode_(const string_view_t& color)
    {
        fwrite(color.data(), sizeof(char), color.size(), file_);
    }

    void print_range_(std::size_t begin, std::size_t end, const memory_buf_t& buf)
    {
        fwrite(buf.data() + begin, sizeof(char), end - begin, file_);
    }

    static std::string to_string_(const string_view_t& view)
    {
        return std::string(view.data(), view.size());
    }
    
private:
    std::FILE* file_;
    mutex_t& mutex_;
    std::unique_ptr<formatter> formatter_;
    std::array<std::string, level::n_levels> colors_;
};

template <typename ConsoleMutex>
class stdout_color_sink : public stdout_color_sink_base<ConsoleMutex>
{
public:
    stdout_color_sink()
        : stdout_color_sink_base<ConsoleMutex>(stdout)
    {}
};

template <typename ConsoleMutex>
class stderr_color_sink : public stdout_color_sink_base<ConsoleMutex>
{
public:
    stderr_color_sink()
        : stdout_color_sink_base<ConsoleMutex>(stdout)
    {}
};

using stdout_color_sink_st = stdout_color_sink<details::console_nullmutex>;
using stdout_color_sink_mt = stdout_color_sink<details::console_mutex>;

using stderr_color_sink_st = stderr_color_sink<details::console_nullmutex>;
using stderr_color_sink_mt = stderr_color_sink<details::console_mutex>;

} // namespace sinks

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stdout_color_logger_st(std::string logger_name)
{
    return Factory::template create<sinks::stdout_color_sink_st>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stdout_color_logger_mt(std::string logger_name)
{
    return Factory::template create<sinks::stdout_color_sink_mt>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stderr_color_logger_st(std::string logger_name)
{
    return Factory::template create<sinks::stderr_color_sink_st>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stderr_color_logger_mt(std::string logger_name)
{
    return Factory::template create<sinks::stderr_color_sink_mt>(std::move(logger_name));
}

} // namespace mylog
