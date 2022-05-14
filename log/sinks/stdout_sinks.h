#pragma once

#include "log/details/log_msg.h"
#include "log/details/console_global.h"
#include "log/synchronous_factory.h"
#include "log/sinks/sink.h"
#include "log/common.h"
#include "log/pattern_formatter.h"

namespace mylog {
namespace sinks {

template<typename ConsoleMutex>    
class stdout_sink_base : public sink
{
public:
    using mutex_t = typename ConsoleMutex::mutex_t;
    explicit stdout_sink_base(std::FILE* file)
        : mutex_(ConsoleMutex::mutex())
        , file_(file)
        , formatter_(std::make_unique<pattern_formatter>())
    {}
    ~stdout_sink_base() = default;

    stdout_sink_base(const stdout_sink_base&) = delete;
    stdout_sink_base& operator=(const stdout_sink_base&) = delete;
    
    stdout_sink_base(stdout_sink_base&&) = delete;
    stdout_sink_base& operator=(stdout_sink_base&&) = delete;
    

    void log(const details::log_msg& msg) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        memory_buf_t buf;
        formatter_->format(msg, buf);
        fwrite(buf.data(), sizeof(char), buf.size(), file_);
        std::fflush(file_); // flush every line to terminal
    }
    
    void flush() override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        std::fflush(file_);
    }
    
    void set_pattern(const std::string& pattern) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        // formatter_ = std::unique_ptr<mylog::formatter>(new pattern_formatter(pattern));
        formatter_ = std::make_unique<pattern_formatter>(pattern);
    }
    
    void set_formatter(std::unique_ptr<mylog::formatter> sink_formatter) override
    {
        std::lock_guard<mutex_t> lock(mutex_);
        formatter_ = std::move(sink_formatter);
    }

private:
    mutex_t& mutex_;
    std::FILE* file_;
    std::unique_ptr<formatter> formatter_;
};


template<typename ConsoleMutex>
class stdout_sink : public stdout_sink_base<ConsoleMutex>
{
public:
    stdout_sink()
        : stdout_sink_base<ConsoleMutex>(stdout)
    {}
};

template<typename ConsoleMutex>
class stderr_sink : public stdout_sink_base<ConsoleMutex>
{
public:
    stderr_sink()
        : stdout_sink_base<ConsoleMutex>(stdout)
    {}
};


using stdout_sink_st = stdout_sink<details::console_mutex>;
using stdout_sink_mt = stdout_sink<details::console_nullmutex>;

using stderr_sink_st = stdout_sink<details::console_mutex>;
using stderr_sink_mt = stdout_sink<details::console_nullmutex>;

} // namespace sinks


template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stdout_logger_st(std::string logger_name)
{
    return Factory::template create<sinks::stdout_sink_st>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stdout_logger_mt(std::string logger_name)
{
    return Factory::template create<sinks::stdout_sink_mt>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stderr_logger_st(std::string logger_name)
{
    return Factory::template create<sinks::stderr_sink_st>(std::move(logger_name));
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> stderr_logger_mt(std::string logger_name)
{
    return Factory::template create<sinks::stderr_sink_mt>(std::move(logger_name));
}


} // namespace mylog
