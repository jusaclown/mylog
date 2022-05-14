#include "log/mylog.h"
#include "log/pattern_formatter.h"

namespace mylog {

void register_logger(std::shared_ptr<logger> new_logger)
{
    details::registry::instance().register_logger(std::move(new_logger));
}

void initialize_logger(std::shared_ptr<logger> new_logger)
{
    details::registry::instance().initialize_logger(std::move(new_logger));
}

void set_default_logger(std::shared_ptr<logger> new_default_logger)
{
    details::registry::instance().set_default_logger(std::move(new_default_logger));
}

std::shared_ptr<logger> default_logger()
{
    return details::registry::instance().default_logger();
}

logger* get_default_raw()
{
    return details::registry::instance().get_default_raw();
}

std::shared_ptr<logger> get(const std::string& logger_name)
{
    return details::registry::instance().get(logger_name);
}

level::level_enum get_level()
{
    return get_default_raw()->level();
}

bool should_log(level::level_enum lvl)
{
    return get_default_raw()->should_log(lvl);
}

void set_level(level::level_enum lvl)
{
    details::registry::instance().set_level(lvl);
}

void set_formatter(std::unique_ptr<formatter> new_formatter)
{
    details::registry::instance().set_formatter(std::move(new_formatter));
}

void set_pattern(std::string pattern)
{
    set_formatter(std::make_unique<pattern_formatter>(std::move(pattern)));
}

void set_flush_level(level::level_enum log_level)
{
    details::registry::instance().set_flush_level(log_level);
}

void flush_every(std::chrono::seconds interval)
{
    details::registry::instance().flush_every(interval);
}

void set_error_handler(void (*handler)(const std::string &msg))
{
    details::registry::instance().set_error_handler(handler);
}

// Automatic registration of loggers when using mylog::create() or mylog::create_async
void set_automatic_registration(bool flag)
{
    details::registry::instance().set_automatic_registration(flag);
}

// Apply a user defined function on all registered loggers
// Example:
// mylog::apply_all([&](std::shared_ptr<mylog::logger> l) {l->flush();});
void apply_all(const std::function<void(const std::shared_ptr<logger>)>& func)
{
    details::registry::instance().apply_all(func);
}

// Drop the reference to the given logger
void drop(const std::string& logger_name)
{
    details::registry::instance().drop(logger_name);
}

// Drop all references from the registry
void drop_all()
{
    details::registry::instance().drop_all();
}

// stop any running threads started by mylog and clean registry loggers
void shutdown()
{
    details::registry::instance().shutdown();
}


} // namespace mylog
