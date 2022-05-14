#pragma once 

#include "log/details/circular_q.h"

#include <mutex>
#include <condition_variable>

namespace mylog {
namespace details {

template <typename T>
class mpmc_blocking_queue
{
public:
    using item_type = T;
    explicit mpmc_blocking_queue(std::size_t max_size)
        : q_(max_size)
    {}

    // try to enqueue and block if no room left
    void enqueue(T&& val)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            pop_cv_.wait(lock, [this] { return !this->q_.full(); });
            q_.push_back(std::move(val));
        }
        push_cv_.notify_one();
    }

    // enqueue immediately. overrun oldest message in the queue if no room left.
    void enqueue_nowait(T&& val)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            q_.push_back(std::move(val));
        }
        push_cv_.notify_one();
    }

    // try to dequeue item. if no item found. wait up to timeout and try again
    // Return true, if succeeded dequeue item, false otherwise
    bool dequeue_for(T& popped_item, std::chrono::milliseconds wait_duration)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!push_cv_.wait_for(lock, wait_duration, [this] {return !this->q_.empty();}))
            {
                return false;
            }
            popped_item = std::move(q_.front());
            q_.pop_front();
        }
        pop_cv_.notify_one();
        return true;
    }

    std::size_t size()
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return q_.size();
    }
    
    std::size_t overrun_counter()
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return q_.overrun_counter();
    }

private:
    std::mutex queue_mutex_;
    std::condition_variable pop_cv_, push_cv_;
    circular_q<T> q_;
};

} // namespace details
} // namespace mylog
