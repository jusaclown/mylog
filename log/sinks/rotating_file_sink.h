#pragma once

#include "log/sinks/base_sink.h"
#include "log/details/file_helper.h"
#include "log/common.h"
#include "log/details/console_global.h"
#include "log/synchronous_factory.h"
#include "log/details/os.h"

namespace mylog {
namespace sinks {

// Rotating file sink based on size
template<typename Mutex>    
class rotating_file_sink : public base_sink<Mutex>
{
public:
    rotating_file_sink(filename_t filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open = false);
    ~rotating_file_sink() = default;

    static filename_t calc_filename(const filename_t& filename, std::size_t index);
    filename_t filename();
    
protected:
    void sink_it_(const details::log_msg& msg) override;
    void flush_() override;

private:
    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    void rotate_();

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file_(const filename_t& src_filename, const filename_t& target_filename);

private:
    filename_t base_filename_;
    std::size_t max_size_;
    std::size_t max_files_;
    std::size_t current_size_;
    details::file_helper file_helper_;
};


template<typename Mutex>
inline rotating_file_sink<Mutex>::rotating_file_sink(filename_t filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open)
    : base_filename_(filename)
    , max_size_(max_size)
    , max_files_(max_files)
    , current_size_(0)
{
    if (max_size == 0)
    {
        throw_mylog_ex("rotating sink constructor: max_size arg cannot be zero");
    }

    if (max_files > 200000)
    {
        throw_mylog_ex("rotating sink constructor: max_files arg cannot exceed 200000");
    }
    
    file_helper_.open(base_filename_);
    current_size_ = file_helper_.size(); // expensive. called only once
    if (rotate_on_open && current_size_ > 0)
    {
        rotate_();
        current_size_ = 0;
    }
}

template<typename Mutex> 
inline filename_t rotating_file_sink<Mutex>::calc_filename(const filename_t& filename, std::size_t index)
{
    if (index == 0u)
        return filename;
    
    filename_t base_name, ext_name;
    
    std::tie(base_name, ext_name) = details::file_helper::split_by_extension(filename);
    return fmt::format("{}.{}{}", base_name, index, ext_name);
}

template<typename Mutex> 
inline filename_t rotating_file_sink<Mutex>::filename()
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return file_helper_.filename();
}

template<typename Mutex> 
inline void rotating_file_sink<Mutex>::sink_it_(const details::log_msg& msg)
{
    memory_buf_t buf;
    base_sink<Mutex>::formatter_->format(msg, buf);
    auto new_size = current_size_ + buf.size();

    if (new_size > max_size_)
    {
        file_helper_.flush();
        if (file_helper_.size() > 0)
        {
            rotate_();
            new_size = buf.size();
        }
    }
    
    file_helper_.write(buf);
    current_size_ = new_size;
}

template<typename Mutex> 
inline void rotating_file_sink<Mutex>::flush_()
{
    file_helper_.flush();
}

template<typename Mutex> 
inline void rotating_file_sink<Mutex>::rotate_()
{
    using details::os::filename_to_str;
    using details::os::path_exists;
    
    file_helper_.close();
    
    for (auto index = max_files_; index > 0; --index)
    {
        filename_t src = calc_filename(base_filename_, index - 1);
        if (!path_exists(src))
        {
            continue;
        }
        filename_t target = calc_filename(base_filename_, index);
        
        if (!rename_file_(src, target))
        {
            details::os::sleep_for_millis(100);
            if (!rename_file_(src, target))
            {
                file_helper_.reopen(true);
                current_size_ = 0;
                throw_mylog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
            }
        }
    }
    file_helper_.reopen(true);
}

template<typename Mutex> 
inline bool rotating_file_sink<Mutex>::rename_file_(const filename_t& src_filename, const filename_t& target_filename)
{
    // 文件不存在会返回-1
    (void)std::remove(target_filename.c_str());
    return std::rename(src_filename.c_str(), target_filename.c_str()) == 0;
}


using rotating_file_sink_mt = rotating_file_sink<std::mutex>;
using rotating_file_sink_st = rotating_file_sink<details::null_mutex>;

} // namespace sinks

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> rotating_logger_mt(std::string logger_name, filename_t filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open = false)
{
    return Factory::template create<sinks::rotating_file_sink_mt>(std::move(logger_name), std::move(filename), max_size, max_files, rotate_on_open);
}

template<typename Factory = synchronous_factory>
inline std::shared_ptr<logger> rotating_logger_st(std::string logger_name, filename_t filename, std::size_t max_size, std::size_t max_files, bool rotate_on_open = false)
{
    return Factory::template create<sinks::rotating_file_sink_st>(std::move(logger_name), std::move(filename), max_size, max_files, rotate_on_open);
}

} // namespace mylog