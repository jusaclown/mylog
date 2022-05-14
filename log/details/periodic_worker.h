#pragma once

#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <chrono>

namespace mylog {
namespace details {

class periodic_worker
{
public:
    periodic_worker(const std::function<void()>& cb_fun, std::chrono::seconds interval);
    ~periodic_worker();
    
    periodic_worker(const periodic_worker&) = delete;
    periodic_worker& operator=(const periodic_worker&) = delete;
    
private:
    bool active_;
    std::thread worker_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

inline periodic_worker::periodic_worker(const std::function<void()>& cb_fun, std::chrono::seconds interval)
{
    active_ = (interval > std::chrono::seconds::zero());
    if (!active_)
    {
        return;
    }
    
    worker_thread_ = std::thread([this, cb_fun, interval]() {
        while (true)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            if (this->cv_.wait_for(lock, interval, [this] {return !this->active_;}))
            {
                return;
            }
            cb_fun();
        }
    });
}

inline periodic_worker::~periodic_worker()
{
    if (worker_thread_.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_ = false;
        }
        cv_.notify_one();
        worker_thread_.join();
    }
}

} // namespace details
} // namespace mylog
