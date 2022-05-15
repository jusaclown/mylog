#pragma once

#include "log/common.h"

#include <fmt/core.h>
#include <fmt/format.h>

namespace mylog {
namespace details {

inline string_view_t to_string_view(const memory_buf_t& buf) noexcept
{
    return string_view_t{ buf.data(), buf.size() };
}

inline void append_string_view(const string_view_t& view, memory_buf_t& dest)
{
    auto* ptr = view.data();
    dest.append(ptr, ptr + view.size());
}

template<typename T>
inline void append_int(T n, memory_buf_t &dest)
{
    fmt::format_int i(n);
    dest.append(i.data(), i.data() + i.size());
}


inline void pad2(int n, memory_buf_t& dest)
{
    if (n >= 0 && n < 100)
    {
        dest.push_back(static_cast<char>('0' + n / 10));
        dest.push_back(static_cast<char>('0' + n % 10));
    }
    else
    {
        fmt::format_to(std::back_inserter(dest), "{:02}", n);
    }
}

template<typename T>
inline void pad3(T n, memory_buf_t& dest)
{
    static_assert(std::is_unsigned<T>::value, "pad3 must get unsigned T");
    if (n < 1000) {
        dest.push_back(static_cast<char>('0' + n / 100));
        n = n % 100;
        dest.push_back(static_cast<char>('0' + n / 10));
        dest.push_back(static_cast<char>('0' + n % 10));
    }
    else
    {
        append_int(n, dest);
    }
}

template<typename T>
inline unsigned int count_digits(T n)
{
    using count_type = typename std::conditional<(sizeof(T) > sizeof(uint32_t)), uint64_t, uint32_t>::type;
    return static_cast<unsigned int>(fmt::detail::count_digits(static_cast<count_type>(n)));
}

template<typename T>
inline void pad_uint(T n, unsigned int width, memory_buf_t& dest)
{
    static_assert(std::is_unsigned<T>::value, "pad_uint must get unsigned T");
    for (auto digits = count_digits(n); digits < width; digits++)
    {
        dest.push_back('0');
    }
    append_int(n, dest);
}


template<typename T>
inline void pad6(T n, memory_buf_t &dest)
{
    pad_uint(n, 6, dest);
}

template<typename T>
inline void pad9(T n, memory_buf_t &dest)
{
    pad_uint(n, 9, dest);
}

template<typename ToDuration>
inline ToDuration time_fraction(log_clock::time_point tp)
{
    using std::chrono::seconds;
    using std::chrono::duration_cast;

    auto duration = tp.time_since_epoch();
    auto secs = duration_cast<seconds>(duration);
    return duration_cast<ToDuration>(duration) - duration_cast<ToDuration>(secs);
}

} // namespace details
} // namespace mylog