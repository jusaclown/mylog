#pragma once

#include "log/details/log_msg.h"

namespace mylog {

class formatter
{
public:
    virtual ~formatter() = default;
    virtual void format(const details::log_msg& msg, memory_buf_t& dest) = 0;
    virtual std::unique_ptr<formatter> clone() const = 0;
};

    
} // namespace mylog
