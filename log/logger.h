#pragma once

#include "log/common.h"
#include "log/level.h"
#include "log/sinks/sink.h"

#include <vector>
#include <string>

#define MYLOG_LOGGER_CATCH(location)                                                                                                   \
    catch (const std::exception &ex)                                                                                                   \
    {                                                                                                                                  \
        if (!location.empty())                                                                                                         \
        {                                                                                                                              \
            err_handler_(fmt::format("{} [{}({})]", ex.what(), location.filename, location.line));                                     \
        }                                                                                                                              \
        else                                                                                                                           \
        {                                                                                                                              \
            err_handler_(ex.what());                                                                                                   \
        }                                                                                                                              \
    }                                                                                                                                  \
    catch (...)                                                                                                                        \
    {                                                                                                                                  \
        err_handler_("Rethrowing unknown exception in logger");                                                                        \
        throw;                                                                                                                         \
    }

namespace mylog {
class logger
{
public:
    // Empty logger
    explicit logger(std::string name)
        : name_(std::move(name))
        , sinks_()
    {}

    // Logger with range on sinks
    template<typename It>
    logger(std::string name, It begin, It end)
        : name_(std::move(name))
        , sinks_(begin, end)
    {}

    // Logger with sinks init list
    logger(std::string name, sinks_init_list sinks)
        : logger(std::move(name), sinks.begin(), sinks.end())
    {}

    // Logger with single sink
    logger(std::string name, sink_ptr sink)
        : logger(std::move(name), {std::move(sink)})
    {}

    virtual ~logger() = default;
    
    logger(const logger& other);
    logger(logger&& other);
    logger& operator=(logger other);
    void swap(logger& other);


    // A compile-time error because 'd' is an invalid specifier for strings.
    // std::string s = fmt::format(FMT_STRING("{:d}"), "foo");
    // fmt::format_string<Args...>fmt 编译时检查，大概？
    template<typename... Args>
    void log(source_loc loc, level::level_enum lvl, fmt::format_string<Args...> fmt, Args&&... args)
    {
        log_(loc, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void log(level::level_enum lvl, fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(source_loc{}, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename T>
    void log(level::level_enum lvl, const T& msg)
    {
        log(source_loc{}, lvl, msg);
    }

    void log(log_clock::time_point log_time, source_loc loc, level::level_enum lvl, string_view_t msg)
    {
        if (!should_log(lvl))
        {
            return;
        }

        try
        {
            details::log_msg logmsg(log_time, loc, name_, lvl, msg);
            sink_it_(logmsg);
        }
        MYLOG_LOGGER_CATCH(loc)
    }

    void log(source_loc loc, level::level_enum lvl, string_view_t msg)
    {
        if (!should_log(lvl))
        {
            return;
        }

        try
        {
            details::log_msg logmsg(loc, name_, lvl, msg);
            sink_it_(logmsg);
        }
        MYLOG_LOGGER_CATCH(loc)
    }

    void log(level::level_enum lvl, string_view_t msg)
    {
        log(source_loc{}, lvl, msg);
    }

    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::warning, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::error, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void fatal(fmt::format_string<Args...> fmt, Args&&... args)
    {
        log(level::fatal, fmt, std::forward<Args>(args)...);
    }

    template<typename T> void trace(const T& msg)   { log(level::trace, msg);   }
    template<typename T> void debug(const T& msg)   { log(level::debug, msg);   }
    template<typename T> void info(const T& msg)    { log(level::info, msg);    }
    template<typename T> void warning(const T& msg) { log(level::warning, msg); }
    template<typename T> void error(const T& msg)   { log(level::error, msg);   }
    template<typename T> void fatal(const T& msg)   { log(level::fatal, msg);   }


    bool should_log(level::level_enum lvl) const;

    void set_formatter(std::unique_ptr<formatter> f);
    void set_pattern(std::string pattern);

    // name
    const std::string& name() const;

    // sinks
    const std::vector<sink_ptr> &sinks() const;
    std::vector<sink_ptr> &sinks();

    // level
    void set_level(level::level_enum lvl);
    level::level_enum level() const;

    // flush functions
    void flush();
    void set_flush_level(level::level_enum log_level);
    level::level_enum flush_level() const;
    
    // error handler
    void set_error_handler(err_handler);

    // create new logger with same sinks and configuration.
    virtual std::shared_ptr<logger> clone(std::string logger_name);

protected:
    template<typename... Args>
    void log_(source_loc loc, level::level_enum lvl, string_view_t fmt, Args&&... args)
    {
        if (!should_log(lvl))
        {
            return;
        }

        try
        {
            memory_buf_t buf;
            fmt::detail::vformat_to(buf, fmt, fmt::make_format_args(std::forward<Args>(args)...));
            details::log_msg msg(loc, name_, lvl, string_view_t(buf.data(), buf.size()));
            sink_it_(msg);
        }
        MYLOG_LOGGER_CATCH(loc)
    }

    void err_handler_(const std::string& msg);
    bool should_flush_(const details::log_msg& msg);
    
    virtual void sink_it_(const details::log_msg& msg);
    virtual void flush_();

protected:
    std::string name_;
    std::vector<sink_ptr> sinks_;
    level_t level_{ level::info };
    level_t flush_level_{ level::fatal };
    err_handler custom_err_handler_{nullptr};
};

inline void swap(logger& a, logger& b)
{
    a.swap(b);
}

} // namespace mylog
