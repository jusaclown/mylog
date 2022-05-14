#pragma once

#include "log/common.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mylog {
namespace details {
namespace os {

inline std::tm localtime(const std::time_t &time_tt) noexcept
{

    std::tm tm;
    ::localtime_r(&time_tt, &tm);
    return tm;
}

inline std::tm localtime() noexcept
{
    std::time_t now_t = ::time(nullptr);
    return localtime(now_t);
}
    
inline std::size_t thread_id() noexcept
{
    static thread_local const std::size_t tid = static_cast<std::size_t>(::syscall(SYS_gettid));
    return tid;
}

inline const char* basename(const char* filename)
{
    const char* res = std::strrchr(filename, '/');
    return res == nullptr ? filename : res + 1;
}

inline void sleep_for_millis(unsigned int millis)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

inline std::string filename_to_str(const filename_t& filename)
{
    return filename;
}

inline std::size_t filesize(std::FILE* fp)
{
    if (fp == nullptr)
    {
        throw_mylog_ex("Failed getting file size. fd is null");
    }
    
    struct stat64 stat_buf;
    if (fstat64(fileno(fp), &stat_buf) == 0)
    {
        return static_cast<std::size_t> (stat_buf.st_size);
    }
    else
    {
        throw_mylog_ex("Failed getting file size", errno);
    }
    return 0;
}

/*
    aaa/bbb/ccc.txt  -> aaa/bbb
    aaa.txt -> ""
*/
inline filename_t dirname(const filename_t& filename)
{
    auto pos = filename.find_last_of('/');
    return pos != filename.npos ? filename.substr(0, pos) : filename_t{};
}

inline bool path_exists(const filename_t& filename)
{
    struct stat buffer;
    return (::stat(filename.c_str(), &buffer) == 0);
}

static inline bool mkdir_(const filename_t& filename)
{
    return ::mkdir(filename.c_str(), static_cast<mode_t>(0755)) == 0;
}

inline bool create_dir(const filename_t& filename)
{
    if (path_exists(filename))
        return true;

    if (filename.empty())
        return false;

    std::size_t offset = 0;
    do
    {
        // aaa/bbb/ccc
        auto index = filename.find_first_of('/', offset);
        if (index == filename_t::npos)
        {
            index = filename.size();
        }

        auto sub_dir = filename.substr(0, index);

        // 创建出错了就返回
        if (!sub_dir.empty() && !path_exists(sub_dir) && !mkdir_(sub_dir))
        {
            return false;
        }

        offset = index + 1;
    } while (offset < filename.size());

    return true;
}

inline int remove_if_exists(const filename_t& filename)
{
    if (path_exists(filename))
        return std::remove(filename.c_str());
    else
        return 0;
}

} // namespace os
} // namespace details
} // namespace mylog
