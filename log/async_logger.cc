#include "log/async_logger.h"
#include "log/details/thread_pool.h"

namespace mylog {

        
std::shared_ptr<logger> async_logger::clone(std::string new_name)
{
    auto cloned = std::make_shared<async_logger>(*this);
    cloned->name_ = std::move(new_name);
    return cloned;
}

void async_logger::sink_it_(const details::log_msg& msg)
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        pool_ptr->post_log(shared_from_this(), msg, overflow_policy_);
    }
    else
    {
        throw_mylog_ex("async log: thread pool doesn't exist anymore");
    }
}

void async_logger::flush_()
{
    if (auto pool_ptr = thread_pool_.lock())
    {
        pool_ptr->post_flush(shared_from_this(), overflow_policy_);
    }
    else
    {
        throw_mylog_ex("async log: thread pool doesn't exist anymore");
    }
}

/* backend functions - called from the thread pool to do the actual job */
void async_logger::backend_sink_it_(const details::log_msg& msg)
{
    for (auto& s : sinks_)
    {
        if (s->should_log(msg.level))
        {
            try
            {
                s->log(msg);
            }
            MYLOG_LOGGER_CATCH(msg.source)
        }
    }

    if (should_flush_(msg))
    {
        backend_flush_();
    }

}

void async_logger::backend_flush_()
{
    for (auto& s : sinks_)
    {
        try
        {
            s->flush();
        }
        MYLOG_LOGGER_CATCH(source_loc{});
    }
}

    
} // namespace mylog
