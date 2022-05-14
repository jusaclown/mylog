#pragma once

#include "log/logger.h"
#include "log/details/registry.h"

#include <memory>

namespace mylog {
struct synchronous_factory
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<logger> create(std::string logger_name, SinkArgs&&... args)
    {
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<logger>(std::move(logger_name), std::move(sink));
        details::registry::instance().initialize_logger(new_logger);
        return new_logger;
    }

};

} // namespace mylog
