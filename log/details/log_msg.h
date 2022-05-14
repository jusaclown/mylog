#pragma once

#include "log/common.h"
#include "log/level.h"



namespace mylog {
namespace details {

// [年-月-日 时-分-秒.毫秒] [日志级别] [线程id] [文件:行号 函数] 日志消息
struct log_msg
{
    log_msg() = default;
    log_msg(log_clock::time_point time_, source_loc loc_, const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_);
    log_msg(source_loc loc_, const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_);
    log_msg(const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_);

    string_view_t logger_name;
    log_clock::time_point time;
    level::level_enum level{ level::off };
    std::size_t thread_id{ 0 };

    mutable std::size_t color_range_start{ 0 };
    mutable std::size_t color_range_end{ 0 };

    source_loc source;
    string_view_t payload;
};


// 异步日志类型
enum class async_msg_type
{
    log,
    flush,
    terminate
};


// Extend log_msg with internal buffer to store its payload.
// This is needed since log_msg holds string_views that points to stack data.
class log_msg_buffer : public log_msg
{
public:
    log_msg_buffer() = default;
    explicit log_msg_buffer(const log_msg& msg);

    log_msg_buffer(const log_msg_buffer& other);
    log_msg_buffer& operator=(const log_msg_buffer& other);
    
    log_msg_buffer(log_msg_buffer&& other);
    log_msg_buffer& operator=(log_msg_buffer&& other);

private:
    void update_string_views();
    
private:
    memory_buf_t buffer_;
};


/* async_msg */
struct async_msg : public log_msg_buffer
{
    async_msg() = default;
    
    /* 三个构造函数对应着三种异步日志的类型 */
    async_msg(async_logger_ptr &&worker, async_msg_type the_type, const details::log_msg &msg);
    async_msg(async_logger_ptr &&worker, async_msg_type the_type);
    explicit async_msg(async_msg_type the_type);
    
    ~async_msg() = default;
    
    // should only be moved in or out of the queue..
    async_msg(const async_msg&) = delete;
    async_msg& operator=(const async_msg&) = delete;

    async_msg(async_msg&&) = default;
    async_msg& operator=(async_msg&&) = default;

    async_logger_ptr worker_ptr;
    async_msg_type msg_type{ async_msg_type::log };
};

    
} // namespace details
} // namespace mylog
