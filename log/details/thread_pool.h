#pragma once

#include "log/details/mpmc_blocking_queue.h"
#include "log/details/log_msg.h"
#include "log/async_logger.h"

#include <thread>
#include <vector>

namespace mylog {
namespace details {

class thread_pool
{
public:
    using item_type = async_msg;
    
    thread_pool(std::size_t q_max_size, std::size_t thread_nums);
    ~thread_pool();

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    
    thread_pool(thread_pool&&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;

    void post_log(async_logger_ptr &&worker_ptr, const log_msg& msg, async_overflow_policy overflow_policy);
    void post_flush(async_logger_ptr &&worker_ptr, async_overflow_policy overflow_policy);

    std::size_t overrun_counter();
    std::size_t queue_size();

private:
    void post_async_msg_(async_msg&&, async_overflow_policy);
    void worker_loop_();
    
    // process next message in the queue
    // return true if this thread should still be active (while no terminate msg
    // was received)
    bool process_next_msg_();
    
private:
    mpmc_blocking_queue<async_msg> q_;
    std::vector<std::thread> threads_;
};

    
} // namespace details
} // namespace mylog