#pragma once

#include "log/level.h"
#include "log/common.h"
#include "log/formatter.h"
#include "log/logger.h"
#include "log/details/periodic_worker.h"
#include "log/details/thread_pool.h"

#include <unordered_map>
#include <string>
#include <mutex>

namespace mylog {
namespace details {

/*
    registry 是一个日志管理类，日志器通过调用registry的方法把自己交由registry管理，
    registry
*/
class registry
{
public:
    using logger_levels = std::unordered_map<std::string, level::level_enum>;
    
    registry(const registry&) = delete;
    registry& operator=(const registry&) = delete;
    
    static registry& instance();

    void register_logger(std::shared_ptr<logger> new_logger);
    void initialize_logger(std::shared_ptr<logger> new_logger);

    /* default_logger */
    void set_default_logger(std::shared_ptr<logger> new_default_logger);
    std::shared_ptr<logger> default_logger() const;
    
    // Return raw ptr to the default logger.
    // To be used directly by the mylog default api (e.g. mylog::info)
    // This make the default API faster, but cannot be used concurrently with set_default_logger().
    // e.g do not call set_default_logger() from one thread while calling mylog::info() from another.
    logger* get_default_raw();
    
    /* get logger */
    std::shared_ptr<logger> get(const std::string& logger_name) const;

    /* global log level */
    void set_level(level::level_enum lvl);
    void set_levels(logger_levels levels, level::level_enum *global_level);

    /* formatter */
    void set_formatter(std::unique_ptr<formatter> new_formatter);

    /* flush functions */
    void set_flush_level(level::level_enum log_level);
    void flush_all();
    void flush_every(std::chrono::seconds interval);

    /* error handler */
    void set_error_handler(err_handler handler);
    
    void set_automatic_registration(bool flag);

    void apply_all(const std::function<void(const std::shared_ptr<logger>)>& func);

    /* drop */
    void drop(const std::string& logger_name);
    void drop_all();
    void shutdown();

    /* thread pool */
    void set_tp(std::shared_ptr<thread_pool> tp);
    std::shared_ptr<thread_pool> get_tp();
    std::recursive_mutex& tp_mutex();
    
private:
    registry();
    ~registry() = default;

    void throw_if_exist_(const std::string& logger_name);
    void register_logger_(std::shared_ptr<logger> new_logger);

private:
    mutable std::mutex logger_map_mutex_, flusher_mutex_;
    std::shared_ptr<mylog::logger> default_logger_;
    std::unordered_map<std::string, std::shared_ptr<mylog::logger>> loggers_;
    logger_levels logger_levels_;
    std::unique_ptr<formatter> formatter_;
    level::level_enum global_log_level_{ level::info };
    level::level_enum global_flush_level_{ level::fatal };
    std::unique_ptr<periodic_worker> periodic_flusher_;
    err_handler err_handler_;
    bool automatic_registration_{ true };
    std::recursive_mutex tp_mutex_;
    std::shared_ptr<thread_pool> tp_;
};

} // namespace details
} // namespace mylog
