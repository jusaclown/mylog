#pragma once

#include "log/sinks/sink.h"
#include "log/formatter.h"
#include "log/pattern_formatter.h"

#include <mutex>

namespace mylog {
namespace sinks {

template <typename Mutex>
class base_sink : public sink
{
public:
    base_sink();
    explicit base_sink(std::unique_ptr<formatter>);
    virtual ~base_sink() = default;

    base_sink(const base_sink&) = delete;
    base_sink& operator=(const base_sink&) = delete;
    
    base_sink(base_sink&&) = delete;
    base_sink &operator=(base_sink &&) = delete;
    
    void log(const details::log_msg& msg) final;
    void flush() final;
    void set_pattern(const std::string& patern) final;
    void set_formatter(std::unique_ptr<mylog::formatter> sink_formatter) final;

protected:
    virtual void sink_it_(const details::log_msg& msg) = 0;
    virtual void flush_() = 0;
    virtual void set_pattern_(const std::string& pattern);
    virtual void set_formatter_(std::unique_ptr<mylog::formatter> sink_formatter);
    
protected:
    Mutex mutex_;
    std::unique_ptr<formatter> formatter_;
};


template<typename Mutex>
inline base_sink<Mutex>::base_sink()
    : formatter_(new pattern_formatter())
{}

template<typename Mutex>
inline base_sink<Mutex>::base_sink(std::unique_ptr<formatter> new_formatter)
    : formatter_(std::move(new_formatter))
{}

template<typename Mutex>
inline void base_sink<Mutex>::log(const details::log_msg& msg)
{
    std::lock_guard<Mutex> lock(mutex_);
    sink_it_(msg);
}

template<typename Mutex>
inline void base_sink<Mutex>::flush()
{
    std::lock_guard<Mutex> lock(mutex_);
    flush_();
}

template<typename Mutex>
inline void base_sink<Mutex>::set_pattern(const std::string& pattern)
{
    std::lock_guard<Mutex> lock(mutex_);
    set_pattern_(pattern);
}

template<typename Mutex>
inline void base_sink<Mutex>::set_formatter(std::unique_ptr<mylog::formatter> sink_formatter)
{
    std::lock_guard<Mutex> lock(mutex_);
    set_formatter_(std::move(sink_formatter));
}

template<typename Mutex>
inline void base_sink<Mutex>::set_pattern_(const std::string& patern)
{
    set_formatter_(std::make_unique<pattern_formatter>());
}

template<typename Mutex>
inline void base_sink<Mutex>::set_formatter_(std::unique_ptr<mylog::formatter> sink_formatter)
{
    formatter_ = std::move(sink_formatter);
}

} // namespace sinks
} // namespace mylog

