#pragma once

#include "log/common.h"
#include "log/level.h"

#include <vector>

namespace mylog {
namespace details {

template<typename T>
class circular_q
{
public:
    using value_type = T;

    circular_q() = default;
    explicit circular_q(std::size_t max_size)
        : max_size_(max_size + 1)
        , q_(max_size_)
    {}
    
    ~circular_q() = default;
    
    circular_q(const circular_q&) = default;
    circular_q &operator=(const circular_q &) = default;

    circular_q(circular_q&& other)
    {
        move_(std::move(other));
    }

    circular_q& operator=(circular_q&& other)
    {
        if (&other != this)
        {
            move_(std::move(other));
        }  

        return *this;
    }

    void push_back(T &&value)
    {
        if (max_size_ > 0)
        {
            q_[tail_] = std::move(value);
            tail_ = (tail_ + 1) % max_size_;

            if (tail_ == head_)
            {
                head_ = (head_ + 1) % max_size_;
                ++overrun_counter_;
            }
        }
    }

    // Pop item from front.
    // If there are no elements in the container, the behavior is undefined
    void pop_front()
    {
        head_ = (head_ + 1) % max_size_;
    }
    
    // Return reference to the front item.
    // If there are no elements in the container, the behavior is undefined.
    const T& front() const
    {
        return q_[head_];
    }

    T& front()
    {
        return q_[head_];
    }

    bool empty() const
    {
        return head_ == tail_;
    }

    bool full() const
    {
        if (max_size_ > 0)
        {
            return ((tail_ + 1) % max_size_) == head_;
        }
        return false;
    }

    // Return number of elements actually stored
    std::size_t size() const
    {
        if (tail_ >= head_)
        {
            return tail_ - head_;
        }
        else
        {
            return max_size_ - head_ + tail_;
        }
    }

    std::size_t overrun_counter() const
    {
        return overrun_counter_;
    }
    
    // Return const reference to item by index.
    // If index is out of range 0…size()-1, the behavior is undefined.
    const T &at(size_t i) const
    {
        assert(i < size());
        return q_[(head_ + i) % max_size_];
    }

private:
    // copy from other&& and reset it to disabled state
    void move_(circular_q&& other)
    {
        max_size_ = other.max_size_;
        q_ = std::move(other.q_);
        head_ = other.head_;
        tail_ = other.tail_;
        overrun_counter_ = other.overrun_counter_;

        // put &&other in disabled, but valid state
        other.max_size_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
        other.overrun_counter_ = 0;
    }

private:
    std::size_t max_size_;
    std::vector<T> q_;
    typename std::vector<T>::size_type head_ = 0;
    typename std::vector<T>::size_type tail_ = 0;
    std::size_t overrun_counter_ = 0;
};

    
} // namespace details
} // namespace mylog
