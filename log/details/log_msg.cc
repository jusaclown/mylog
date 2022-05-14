#include "log/details/log_msg.h"
#include "log/details/os.h"
#include "log/details/fmt_helper.h"

namespace mylog {
namespace details {

/* log_msg */    
log_msg::log_msg(log_clock::time_point time_, source_loc loc_, const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_)
    : logger_name(logger_name_)
    , time(time_)
    , level(level_)
    , thread_id(os::thread_id())
    , source(loc_)
    , payload(payload_)
{}

log_msg::log_msg(source_loc loc_, const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_)
    : log_msg(log_clock::now(), loc_, logger_name_, level_, payload_)
{}

log_msg::log_msg(const string_view_t& logger_name_, level::level_enum level_, const string_view_t& payload_)
    : log_msg(log_clock::now(), source_loc{} , logger_name_, level_, payload_)
{}


/* log_msg_buffer */
log_msg_buffer::log_msg_buffer(const log_msg& msg)
    : log_msg(msg)
{
    buffer_.append(logger_name.begin(), logger_name.end());
    buffer_.append(payload.begin(), payload.end());
    update_string_views();
}

log_msg_buffer::log_msg_buffer(const log_msg_buffer& other)
    : log_msg(other)
{
    buffer_.append(logger_name.begin(), logger_name.end());
    buffer_.append(payload.begin(), payload.end());
    update_string_views();
}

log_msg_buffer& log_msg_buffer::operator=(const log_msg_buffer& other)
{
    if (this == &other)
    {
        return *this;
    }

    log_msg::operator=(other);
    buffer_.clear();
    buffer_.append(other.buffer_.data(), other.buffer_.data() + other.buffer_.size());
    update_string_views();
    return *this;
}

log_msg_buffer::log_msg_buffer(log_msg_buffer&& other)
    : log_msg(other)
    , buffer_(std::move(other.buffer_))
{
    update_string_views();
}

log_msg_buffer& log_msg_buffer::operator=(log_msg_buffer&& other)
{
    if (this == &other)
    {
        return *this;
    }

    log_msg::operator=(other);
    buffer_ = std::move(other.buffer_);
    update_string_views();
    return *this;
}

void log_msg_buffer::update_string_views()
{
    logger_name = string_view_t(buffer_.data(), logger_name.size());
    payload = string_view_t(buffer_.data() + logger_name.size(), payload.size());
}

/* async_msg */
async_msg::async_msg(async_logger_ptr&& worker, async_msg_type the_type, const details::log_msg& msg)
    : log_msg_buffer(msg)
    , worker_ptr(std::move(worker))
    , msg_type(the_type)
{}

async_msg::async_msg(async_logger_ptr&& worker, async_msg_type the_type)
    : log_msg_buffer()
    , worker_ptr(std::move(worker))
    , msg_type(the_type)
{}

async_msg::async_msg(async_msg_type the_type)
    : log_msg_buffer()
    , worker_ptr(nullptr)
    , msg_type(the_type)
{}

} // namespace details
} // namespace mylog
