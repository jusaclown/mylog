#pragma once

#include "log/common.h"
#include "log/level.h"
#include "log/details/log_msg.h"
#include "log/formatter.h"

#include <memory>

namespace mylog {
namespace sinks {

class sink
{
public:
    virtual ~sink() = default;
    virtual void log(const details::log_msg& msg) = 0;
    virtual void flush() = 0;
    virtual void set_pattern(const std::string& pattern) = 0;
    virtual void set_formatter(std::unique_ptr<mylog::formatter> sink_formatter) = 0;

    level::level_enum level() const
    {
        return static_cast<level::level_enum>(level_.load(std::memory_order_relaxed));
    }

    void set_level(level::level_enum lvl)
    {
        level_.store(lvl, std::memory_order_relaxed);
    }

    bool should_log(level::level_enum lvl) const
    {
        return lvl >= level_.load(std::memory_order_relaxed);
    }

private:
    level_t level_{ level::trace };
};

    
} // namespace sinks
} // namespace mylog
