#pragma once

#include "log/sinks/base_sink.h"
#include "log/details/file_helper.h"
#include "log/common.h"
#include "log/details/console_global.h"
#include "log/synchronous_factory.h"

namespace mylog {
namespace sinks {

template<typename Mutex>    
class basic_file_sink : public base_sink<Mutex>
{
public:
    explicit basic_file_sink(filename_t filename, bool truncate = false)
    {
        file_helper_.open(std::move(filename));
    }

    ~basic_file_sink() = default;

    const filename_t& filename() const
    {
        return file_helper_.filename();
    }
    
protected:
    void sink_it_(const details::log_msg& msg) override
    {
        memory_buf_t buf;
        base_sink<Mutex>::formatter_->format(msg, buf);
        file_helper_.write(buf);
    }
    
    void flush_() override
    {
        file_helper_.flush();
    }

private:
    details::file_helper file_helper_;
};

using basic_file_sink_mt = basic_file_sink<std::mutex>;
using basic_file_sink_st = basic_file_sink<details::null_mutex>;

} // namespace sinks

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> basic_logger_mt(std::string logger_name, filename_t filename, bool truncate = false)
{
    return Factory::template create<sinks::basic_file_sink_mt>(std::move(logger_name), std::move(filename), truncate);
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> basic_logger_st(std::string logger_name, filename_t filename, bool truncate = false)
{
    return Factory::template create<sinks::basic_file_sink_st>(std::move(logger_name), std::move(filename), truncate);
}

} // namespace mylog
