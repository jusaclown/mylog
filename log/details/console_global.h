#pragma once

#include <mutex>

namespace mylog {
namespace details {


class null_mutex {
public:
    void lock() const {}
    bool try_lock() const { return true; }
    void unlock() const {}
};

class console_mutex
{
public:
    using mutex_t = std::mutex;
    static mutex_t& mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};

class console_nullmutex
{
public:
    using mutex_t = null_mutex;
    static mutex_t& mutex()
    {
        static mutex_t s_mutex;
        return s_mutex;
    }
};

    
} // namespace details
} // namespace mylog
