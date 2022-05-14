#pragma once

#include "log/logger.h"

namespace mylog {

namespace details {
    class thread_pool;
}

// Async overflow policy - block by default.
enum class  async_overflow_policy
{
    block,          // Block until message can be enqueued
    overrun_oldest  // Discard oldest message in the queue if full when trying to
                    // add new item.
};

class async_logger final : public std::enable_shared_from_this<async_logger>, public logger
{
    friend class details::thread_pool;

public:
    template<typename It>
    async_logger(std::string logger_name, It begin, It end, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block)
        : logger(std::move(logger_name), begin, end)
        , thread_pool_(tp)
        , overflow_policy_(overflow_policy)
    {}

    async_logger(std::string logger_name, sinks_init_list sinks_list, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block)
        : async_logger(std::move(logger_name), sinks_list.begin(), sinks_list.end(), std::move(tp), overflow_policy)
    {}

    async_logger(std::string logger_name, sink_ptr single_sink, std::weak_ptr<details::thread_pool> tp,
        async_overflow_policy overflow_policy = async_overflow_policy::block)
        : async_logger(std::move(logger_name), {std::move(single_sink)}, std::move(tp), overflow_policy)
    {}
        
    std::shared_ptr<logger> clone(std::string new_name) override;

protected:
    void sink_it_(const details::log_msg& msg) override;
    void flush_() override;
    void backend_sink_it_(const details::log_msg &msg);
    void backend_flush_();

private:
    std::weak_ptr<details::thread_pool> thread_pool_;
    async_overflow_policy overflow_policy_;
};


    
} // namespace mylog
