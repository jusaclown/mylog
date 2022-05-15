#include "log/details/thread_pool.h"
#include "log/common.h"

#include <cassert>

namespace mylog {
namespace details {
    
thread_pool::thread_pool(std::size_t q_max_size, std::size_t thread_nums)
    : q_(q_max_size)
{
    if (thread_nums == 0 || thread_nums > 1000)
    {
        throw_mylog_ex("mylog::thread_pool(): invalid threads_n param (valid range is 1-1000)");
    }

    for (std::size_t i = 0; i < thread_nums; ++i)
    {
        threads_.emplace_back([this]() {
            this->worker_loop_();
        });
    }
}

thread_pool::~thread_pool()
{
    try
    {
        for (std::size_t i = 0; i < threads_.size(); ++i)
        {
            post_async_msg_(async_msg(async_msg_type::terminate), async_overflow_policy::block);
        }

        for (auto& t : threads_)
        {
            t.join();
        }
    }
    catch(const std::exception& e)
    {}   
}


void thread_pool::post_log(async_logger_ptr&& worker_ptr, const log_msg& msg, async_overflow_policy overflow_policy)
{
    async_msg post_msg(std::move(worker_ptr), async_msg_type::log, msg);
    post_async_msg_(std::move(post_msg), overflow_policy);
}

void thread_pool::post_flush(async_logger_ptr&& worker_ptr, async_overflow_policy overflow_policy)
{
    post_async_msg_(async_msg(std::move(worker_ptr), async_msg_type::flush), overflow_policy);
}

std::size_t thread_pool::overrun_counter()
{
    return q_.overrun_counter();
}

std::size_t thread_pool::queue_size()
{
    return q_.size();
}

void thread_pool::post_async_msg_(async_msg&& msg, async_overflow_policy overflow_policy)
{
    if (overflow_policy == async_overflow_policy::block)
    {
        q_.enqueue(std::move(msg));
    }
    else
    {
        q_.enqueue_nowait(std::move(msg));
    }
}

void thread_pool::worker_loop_()
{
    while (process_next_msg_()) {}
}

// process next message in the queue
// return true if this thread should still be active (while no terminate msg
// was received)
bool thread_pool::process_next_msg_()
{
    async_msg msg;
    bool dequeued = q_.dequeue_for(msg, std::chrono::seconds(10));
    if (!dequeued)
    {
        return true;
    }

    switch (msg.msg_type)
    {
    case async_msg_type::log:
    {
        msg.worker_ptr->backend_sink_it_(msg);
        return true;
    }
        
    case async_msg_type::flush:
    {
        msg.worker_ptr->backend_flush_();
        return true;
    }

    case async_msg_type::terminate:
    {
        return false;
    }
    
    default: {
        assert(false);
    }
    }

    return true;
}
    
} // namespace details
} // namespace mylog