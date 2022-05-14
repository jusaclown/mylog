#pragma once

#include "log/common.h"

#include <algorithm>

namespace mylog {

#define MYLOG_LEVEL_TRACE     0
#define MYLOG_LEVEL_DEBUG     1
#define MYLOG_LEVEL_INFO      2
#define MYLOG_LEVEL_WARNING   3
#define MYLOG_LEVEL_ERROR     4
#define MYLOG_LEVEL_FATAL     5
#define MYLOG_LEVEL_OFF       6

#ifndef MYLOG_ACTIVE_LEVEL
#   define MYLOG_ACTIVE_LEVEL MYLOG_LEVEL_INFO
#endif

namespace level {
            
enum level_enum : int {
    trace = MYLOG_LEVEL_TRACE,
    debug = MYLOG_LEVEL_DEBUG,
    info = MYLOG_LEVEL_INFO,
    warning = MYLOG_LEVEL_WARNING,
    error = MYLOG_LEVEL_ERROR,
    fatal = MYLOG_LEVEL_FATAL,
    off = MYLOG_LEVEL_OFF,
    n_levels
};

#define MYLOG_LEVEL_TRACE_NAME     mylog::string_view_t("trace", 5)
#define MYLOG_LEVEL_DEBUG_NAME     mylog::string_view_t("debug", 5)
#define MYLOG_LEVEL_INFO_NAME      mylog::string_view_t("info", 4)
#define MYLOG_LEVEL_WARNING_NAME   mylog::string_view_t("warning", 7)
#define MYLOG_LEVEL_ERROR_NAME     mylog::string_view_t("error", 5)
#define MYLOG_LEVEL_FATAL_NAME     mylog::string_view_t("fatal", 5)
#define MYLOG_LEVEL_OFF_NAME       mylog::string_view_t("off", 3)
 

#define MYLOG_LEVEL_NAMES {           \
        MYLOG_LEVEL_TRACE_NAME,       \
        MYLOG_LEVEL_DEBUG_NAME,       \
        MYLOG_LEVEL_INFO_NAME,        \
        MYLOG_LEVEL_WARNING_NAME,     \
        MYLOG_LEVEL_ERROR_NAME,       \
        MYLOG_LEVEL_FATAL_NAME,       \
        MYLOG_LEVEL_OFF_NAME          \
    }


constexpr static string_view_t level_string_views[] MYLOG_LEVEL_NAMES;

// 日志级别数字转文字   3 -> warn
inline const string_view_t& to_string_view(level::level_enum lvl) noexcept
{
    return level_string_views[lvl];
}

// 日志级别文字转数字   warn -> 3
inline level::level_enum from_str(const std::string& lvl_name) noexcept
{
    if (lvl_name == "warn")
        return level::warning;

    if (lvl_name == "err")
        return level::error;

    auto it = std::find(std::begin(level_string_views), std::end(level_string_views), lvl_name);
    if (it != std::end(level_string_views))
    {
        return static_cast<level::level_enum>(it - std::begin(level_string_views));
    }

    return level::off;
}


} // namespace level
} // namespace mylog

