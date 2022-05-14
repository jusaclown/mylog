#pragma once

#include "log/async_logger.h"
#include "log/details/registry.h"
#include "log/details/thread_pool.h"

#include <memory>
#include <mutex>

namespace mylog {
    
namespace details {
    static const size_t default_async_q_size = 8192;
} // namespace details

// async logger factory - creates async loggers backed with thread pool.
// if a global thread pool doesn't already exist, create it with default queue
// size of 8192 items and single thread.
template<async_overflow_policy OverflowPolicy = async_overflow_policy::block>
struct async_factory_impl
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<logger> create(std::string logger_name, SinkArgs&&... args)
    {
        auto& registry_inst = details::registry::instance();

        // create global thread pool if not already exists..
        auto& mutex = registry_inst.tp_mutex();
        std::lock_guard<std::recursive_mutex> tp_lock(mutex);
        auto tp = registry_inst.get_tp();
        if (tp == nullptr)
        {
            tp = std::make_shared<details::thread_pool>(details::default_async_q_size, 1U);
            registry_inst.set_tp(tp);
        }

        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<async_logger>(std::move(logger_name), std::move(sink), std::move(tp), OverflowPolicy);
        registry_inst.initialize_logger(new_logger);
        return new_logger;
    }
};

using async_factory = async_factory_impl<async_overflow_policy::block>;
using async_factory_nonblock = async_factory_impl<async_overflow_policy::overrun_oldest>;

template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<mylog::logger> create_async(std::string logger_name, SinkArgs &&... sink_args)
{
    return async_factory::create<Sink>(std::move(logger_name), std::forward<SinkArgs>(sink_args)...);
}

template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<mylog::logger> create_async_nb(std::string logger_name, SinkArgs &&... sink_args)
{
    return async_factory_nonblock::create<Sink>(std::move(logger_name), std::forward<SinkArgs>(sink_args)...);
}

// set global thread pool.
inline void init_thread_pool(size_t q_size, size_t thread_count)
{
    auto tp = std::make_shared<details::thread_pool>(q_size, thread_count);
    details::registry::instance().set_tp(std::move(tp));
}

// get the global thread pool.
inline std::shared_ptr<mylog::details::thread_pool> thread_pool()
{
    return details::registry::instance().get_tp();
}

} // namespace mylog
