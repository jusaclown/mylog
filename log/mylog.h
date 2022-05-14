#pragma once

#include "log/common.h"
#include "log/details/registry.h"
#include "log/logger.h"
#include "log/synchronous_factory.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace mylog {

using default_factory = synchronous_factory;

// Create and register a logger with a templated sink type
// The logger's level, formatter and flush level will be set according the
// global settings.
//
// Example:
//   mylog::create<daily_file_sink_st>("logger_name", "dailylog_filename", 11, 59);
template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<logger> create(std::string logger_name, SinkArgs&&... args)
{
    return default_factory::create<Sink>(std::move(logger_name), std::forward<SinkArgs>(args)...);
}

// Register the given logger with the given name
void register_logger(std::shared_ptr<logger> new_logger);

// Initialize and register a logger,
// formatter and flush level will be set according the global settings.
//
// Useful for initializing manually created loggers with the global settings.
//
// Example:
//   auto mylogger = std::make_shared<mylog::logger>("mylogger", ...);
//   mylog::initialize_logger(mylogger);
void initialize_logger(std::shared_ptr<logger> new_logger);

// API for using default logger (stdout_color_mt),
// e.g: mylog::info("Message {}", 1);
//
// The default logger object can be accessed using the mylog::default_logger():
// For example, to add another sink to it:
// mylog::default_logger()->sinks().push_back(some_sink);
//
// The default logger can replaced using mylog::set_default_logger(new_logger).
// For example, to replace it with a file logger.
//
// IMPORTANT:
// The default API is thread safe (for _mt loggers), but:
// set_default_logger() *should not* be used concurrently with the default API.
// e.g do not call set_default_logger() from one thread while calling mylog::info() from another.
void set_default_logger(std::shared_ptr<logger> new_default_logger);
std::shared_ptr<logger> default_logger();
logger* get_default_raw();

// Return an existing logger or nullptr if a logger with such name doesn't
// exist.
// example: mylog::get("my_logger")->info("hello {}", "world");
std::shared_ptr<logger> get(const std::string& logger_name);

// Get default logger level
level::level_enum get_level();

// Determine whether the default logger should log messages with a certain level
bool should_log(level::level_enum lvl);

// Set global logging level
void set_level(level::level_enum lvl);

// Set global formatter. Each sink in each logger will get a clone of this object
void set_formatter(std::unique_ptr<formatter> new_formatter);

// Set global format string.
// example: mylog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
void set_pattern(std::string pattern);

// Set global flush level
void set_flush_level(level::level_enum log_level);

// Start/Restart a periodic flusher thread
// Warning: Use only if all your loggers are thread safe!
void flush_every(std::chrono::seconds interval);

// Set global error handler
void set_error_handler(void (*handler)(const std::string &msg));

// Automatic registration of loggers when using mylog::create() or mylog::create_async
void set_automatic_registration(bool flag);

// Apply a user defined function on all registered loggers
// Example:
// mylog::apply_all([&](std::shared_ptr<mylog::logger> l) {l->flush();});
void apply_all(const std::function<void(const std::shared_ptr<logger>)>& func);

// Drop the reference to the given logger
void drop(const std::string& logger_name);

// Drop all references from the registry
void drop_all();

// stop any running threads started by mylog and clean registry loggers
void shutdown();


template<typename... Args>
void trace(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->trace(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->debug(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->info(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void warning(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->warning(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void error(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->error(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void fatal(fmt::format_string<Args...> fmt, Args&&... args)
{
    get_default_raw()->fatal(fmt, std::forward<Args>(args)...);
}

template<typename T> void trace(const T& msg)   { get_default_raw()->trace(msg);   }
template<typename T> void debug(const T& msg)   { get_default_raw()->debug(msg);   }
template<typename T> void info(const T& msg)    { get_default_raw()->info(msg);    }
template<typename T> void warning(const T& msg) { get_default_raw()->warning(msg); }
template<typename T> void error(const T& msg)   { get_default_raw()->error(msg);   }
template<typename T> void fatal(const T& msg)   { get_default_raw()->fatal(msg);   }

} // namespace mylog


#define MYLOG_LOGGER_CALL(logger, level, ...) (logger)->log(mylog::source_loc{__FILE__, __LINE__, __FUNCTION__}, level, __VA_ARGS__)

#if MYLOG_LEVEL_TRACE >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_TRACE(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::trace, __VA_ARGS__)
#   define MYLOG_TRACE(...) MYLOG_LOGGER_TRACE(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_TRACE(logger, ...) (void)0
#   define MYLOG_TRACE(...) (void) 0
#endif

#if MYLOG_LEVEL_DEBUG >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_DEBUG(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::debug, __VA_ARGS__)
#   define MYLOG_DEBUG(...) MYLOG_LOGGER_DEBUG(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_DEBUG(logger, ...) (void)0
#   define MYLOG_DEBUG(...) (void)0
#endif

#if MYLOG_LEVEL_INFO >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_INFO(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::info, __VA_ARGS__)
#   define MYLOG_INFO(...) MYLOG_LOGGER_INFO(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_INFO(logger, ...) (void)0
#   define MYLOG_INFO(...) (void)0
#endif

#if MYLOG_LEVEL_WARNING >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_WARNING(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::warning, __VA_ARGS__)
#   define MYLOG_WARNING(...) MYLOG_LOGGER_WARNING(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_WARNING(logger, ...) (void)0
#   define MYLOG_WARNING(...) (void)0
#endif

#if MYLOG_LEVEL_ERROR >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_ERROR(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::error, __VA_ARGS__)
#   define MYLOG_ERROR(...) MYLOG_LOGGER_ERROR(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_ERROR(logger, ...) (void)0
#   define MYLOG_ERROR(...) (void)0
#endif

#if MYLOG_LEVEL_FATAL >= MYLOG_ACTIVE_LEVEL
#   define MYLOG_LOGGER_FATAL(logger, ...) MYLOG_LOGGER_CALL(logger, mylog::level::fatal, __VA_ARGS__)
#   define MYLOG_FATAL(...) MYLOG_LOGGER_FATAL(mylog::get_default_raw(), __VA_ARGS__)
#else
#   define MYLOG_LOGGER_FATAL(logger, ...) (void)0
#   define MYLOG_FATAL(...) (void)0
#endif