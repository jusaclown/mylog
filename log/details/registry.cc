#include "log/details/registry.h"
#include "log/pattern_formatter.h"
#include "log/sinks/stdout_color_sinks.h"


namespace mylog {
namespace details {

registry::registry()
    : formatter_(new pattern_formatter())
{
    auto color_sink = std::make_shared<sinks::stdout_color_sink_mt>();
    
    const char* default_logger_name = "default_logger";
    default_logger_ = std::make_shared<logger>(default_logger_name, std::move(color_sink));
    loggers_[default_logger_name] = default_logger_;
}
    
registry& registry::instance()
{
    static registry s_instance;
    return s_instance;
}

void registry::register_logger(std::shared_ptr<logger> new_logger)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    register_logger_(std::move(new_logger));
}

void registry::initialize_logger(std::shared_ptr<logger> new_logger)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    new_logger->set_formatter(formatter_->clone());

    if (err_handler_)
    {
        new_logger->set_error_handler(err_handler_);
    }

    auto it = logger_levels_.find(new_logger->name());
    auto new_level = it != logger_levels_.end() ? it->second : global_log_level_;
    new_logger->set_level(new_level);
    
    new_logger->set_flush_level(global_flush_level_);

    if (automatic_registration_)
    {
        register_logger_(std::move(new_logger));
    }
}

void registry::set_default_logger(std::shared_ptr<logger> new_default_logger)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    default_logger_ = std::move(new_default_logger);
}

std::shared_ptr<logger> registry::default_logger() const
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    return default_logger_;
}

logger* registry::get_default_raw()
{
    return default_logger_.get();
}

std::shared_ptr<logger> registry::get(const std::string& logger_name) const
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    auto it = loggers_.find(logger_name);
    return it == loggers_.end() ? nullptr : it->second;
}

void registry::set_level(level::level_enum lvl)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    for (auto& l : loggers_)
    {
        l.second->set_level(lvl);
    }
    global_log_level_ = lvl;
}

void registry::set_levels(logger_levels levels, level::level_enum* global_level)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    logger_levels_ = std::move(levels);
    auto global_level_requested = global_level != nullptr;
    global_log_level_ = global_level_requested ? *global_level : global_log_level_;

    for (auto logger : loggers_)
    {
        auto logger_entry = logger_levels_.find(logger.first);
        if (logger_entry != logger_levels_.end())
        {
            logger.second->set_level(logger_entry->second);
        }
        else if (global_level_requested)
        {
            logger.second->set_level(*global_level);
        }
    }
}

void registry::set_formatter(std::unique_ptr<formatter> new_formatter)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    formatter_ = std::move(new_formatter);
    for (auto& l : loggers_)
    {
        l.second->set_formatter(formatter_->clone());
    }
}

void registry::set_flush_level(level::level_enum lvl)
{
    std::lock_guard<std::mutex> lock(flusher_mutex_);
    for (auto& l : loggers_)
    {
        l.second->set_flush_level(lvl);
    }
    global_flush_level_ = lvl;
}

void registry::flush_all()
{
    std::lock_guard<std::mutex> lock(flusher_mutex_);
    for (auto& l : loggers_)
    {
        l.second->flush();
    }
}

void registry::flush_every(std::chrono::seconds interval)
{
    std::lock_guard<std::mutex> lock(flusher_mutex_);
    auto cb_fun = [this]() { this->flush_all(); };
    periodic_flusher_ = std::make_unique<periodic_worker>(cb_fun, interval);
}

void registry::set_error_handler(err_handler handler)
{
    std::lock_guard<std::mutex> lock(flusher_mutex_);
    for (auto& l : loggers_)
    {
        l.second->set_error_handler(handler);
    }
    err_handler_ = std::move(handler);
}

void registry::set_automatic_registration(bool flag)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    automatic_registration_ = flag;
}

void registry::apply_all(const std::function<void(const std::shared_ptr<logger>)>& func)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    for (auto &l : loggers_)
    {
        func(l.second);
    }
}

void registry::drop(const std::string& logger_name)
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    loggers_.erase(logger_name);
    if (default_logger_ && default_logger_->name() == logger_name)
    {
        default_logger_.reset();
    }
}

void registry::drop_all()
{
    std::lock_guard<std::mutex> lock(logger_map_mutex_);
    loggers_.clear();
    default_logger_.reset();
}

void registry::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(flusher_mutex_);
        periodic_flusher_.reset();
    }

    drop_all();
    
    {
        std::lock_guard<std::recursive_mutex> lock(tp_mutex_);
        tp_.reset();
    }
}

void registry::set_tp(std::shared_ptr<thread_pool> tp)
{
    std::lock_guard<std::recursive_mutex> lock(tp_mutex_);
    tp_ = std::move(tp);
}

std::shared_ptr<thread_pool> registry::get_tp()
{
    std::lock_guard<std::recursive_mutex> lock(tp_mutex_);
    return tp_;
}

void registry::throw_if_exist_(const std::string& logger_name)
{
    if (loggers_.find(logger_name) != loggers_.end())
    {
        throw_mylog_ex("logger with name '" + logger_name + "' already exists");
    }
}

void registry::register_logger_(std::shared_ptr<logger> new_logger)
{
    auto logger_name = new_logger->name();
    throw_if_exist_(logger_name);
    loggers_[logger_name] = new_logger;
}

std::recursive_mutex &registry::tp_mutex()
{
    return tp_mutex_;
}

} // namespace details
} // namespace mylog 
