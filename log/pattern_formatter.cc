#include "log/pattern_formatter.h"
#include "log/common.h"
#include "log/details/fmt_helper.h"
#include "log/details/os.h"
#include "log/level.h"

namespace mylog {

// 全格式  [年-月-日 时-分-秒.毫秒] [日志器名称] [日志级别] [线程id] [文件:行号 函数] 日志消息
class full_formatter : public flag_formatter
{
public:
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        using std::chrono::seconds;
        using std::chrono::duration_cast;

        auto sec = duration_cast<seconds>(msg.time.time_since_epoch());
        if (sec_ != sec || cached_buf_.size() == 0)
        {
            cached_buf_.clear();
            cached_buf_.push_back('[');
            details::append_int(tm_time.tm_year + 1900, cached_buf_);
            cached_buf_.push_back('-');

            details::pad2(tm_time.tm_mon + 1, cached_buf_);
            cached_buf_.push_back('-');

            details::pad2(tm_time.tm_mday, cached_buf_);
            cached_buf_.push_back(' ');

            details::pad2(tm_time.tm_hour, cached_buf_);
            cached_buf_.push_back('-');

            details::pad2(tm_time.tm_min, cached_buf_);
            cached_buf_.push_back('-');

            details::pad2(tm_time.tm_sec, cached_buf_);
            cached_buf_.push_back('.');

            sec_ = sec;
        }

        dest.append(cached_buf_.begin(), cached_buf_.end());
        
        auto millis = details::time_fraction<std::chrono::milliseconds>(msg.time);
        details::pad3(static_cast<uint32_t>(millis.count()), dest);
        dest.push_back(']');
        dest.push_back(' ');

        if (msg.logger_name.size() > 0)
        {
            dest.push_back('[');
            details::append_string_view(msg.logger_name, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }

        dest.push_back('[');
        msg.color_range_start = dest.size();
        details::append_string_view(level::to_string_view(msg.level), dest);
        msg.color_range_end = dest.size();
        dest.push_back(']');
        dest.push_back(' ');

        if (msg.thread_id != 0)
        {
            dest.push_back('[');
            details::pad6(msg.thread_id, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }

        if (!msg.source.empty())
        {
            dest.push_back('[');
            details::append_string_view(details::os::basename(msg.source.filename), dest);
            dest.push_back(':');
            details::append_int(msg.source.line, dest);
            
            dest.push_back(' ');
            details::append_string_view(msg.source.funname, dest);
            dest.push_back(']');
            dest.push_back(' ');
        }

        details::append_string_view(msg.payload, dest);
    }

private:
    std::chrono::seconds sec_{ 0 };
    memory_buf_t cached_buf_;
};

// 日志消息 %m
class message_formatter : public flag_formatter
{
public:
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        details::append_string_view(msg.payload, dest);
    }
};

// 日志等级 %l
class level_formatter : public flag_formatter
{
public:
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        details::append_string_view(level::to_string_view(msg.level), dest);
    }
};

// 颜色开始 %^
class color_start_formatter : public flag_formatter
{
public:
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        msg.color_range_start = dest.size();
    }
};

// 颜色结束 %$
class color_end_formatter : public flag_formatter
{
public:
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        msg.color_range_end = dest.size();
    }
};

// 单个字符
class char_formatter : public flag_formatter
{
public:
    explicit char_formatter(char ch)
        : ch_(ch)
    {}

    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        dest.push_back(ch_);
    }
    
private:
    char ch_;
};

// 除了%后面的字符的其他字符
class aggregate_formatter : public flag_formatter
{
public:
    aggregate_formatter() = default;
    
    void format(const details::log_msg& msg, const std::tm& tm_time, memory_buf_t& dest) override
    {
        details::append_string_view(str_, dest);
    }

    void add_char(const char ch)
    {
        str_ += ch;
    }

private:
    std::string str_;
};

pattern_formatter::pattern_formatter()
    : pattern_("%+")
{
    std::memset(&cached_tm_, 0, sizeof(cached_tm_));
    formatters_.emplace_back(std::make_unique<full_formatter>());
}

pattern_formatter::pattern_formatter(std::string pattern)
    : pattern_(std::move(pattern))
{
    std::memset(&cached_tm_, 0, sizeof(cached_tm_));
    compile_pattern_();
}

// format 用于将格式化log_msg后的日志消息存入dest中
void pattern_formatter::format(const details::log_msg& msg, memory_buf_t& dest)
{
    // 由于要用到tm格式的时间信息，所以为了减少调用localtime_t的次数，缓存当前的秒和tm
    // 只有当秒不同时才调用
    using std::chrono::seconds;
    using std::chrono::duration_cast;

    auto sec = duration_cast<seconds>(msg.time.time_since_epoch());
    if (sec != last_secs_)
    {
        cached_tm_ = details::os::localtime(log_clock::to_time_t(msg.time));
        last_secs_ = sec;
    }

    for (auto& f : formatters_)
    {
        f->format(msg, cached_tm_, dest);
    }

    dest.push_back('\n');
}

std::unique_ptr<formatter> pattern_formatter::clone() const
{
    return std::make_unique<pattern_formatter>(pattern_);
}

void pattern_formatter::set_pattern(std::string pattern)
{
    pattern_ = std::move(pattern);
    compile_pattern_();
}

void pattern_formatter::handle_flag_(const char ch)
{
    switch (ch)
    {
    case '+':   // default formatter
        formatters_.emplace_back(std::make_unique<full_formatter>());
        break;
    
    case 'v':   // level
        formatters_.emplace_back(std::make_unique<message_formatter>());
        break;

    case 'l':   // the message text
        formatters_.emplace_back(std::make_unique<level_formatter>());
        break;

    case '^':   // color range start
        formatters_.emplace_back(std::make_unique<color_start_formatter>());
        break;
        
    case '$':   // color range end
        formatters_.emplace_back(std::make_unique<color_end_formatter>());
        break;
        
    case '%':
        formatters_.emplace_back(std::make_unique<char_formatter>('%'));
        break;
        
    default:    // Unknown flag appears as is
        
        auto unknown_flag = std::make_unique<aggregate_formatter>();

        unknown_flag->add_char('%');
        unknown_flag->add_char(ch);
        formatters_.push_back((std::move(unknown_flag)));
        
        break;
    }
}

void pattern_formatter::compile_pattern_()
{
    auto cend = pattern_.cend();
    std::unique_ptr<aggregate_formatter> user_chars;
    formatters_.clear();
    
    for (auto it = pattern_.cbegin(); it != cend; ++it)
    {
        // 如果遇到%，则说明后面是一个模式字符
        if (*it == '%')
        {
            if (user_chars)
            {
                formatters_.emplace_back(std::move(user_chars));
            }
            
            if (++it != cend)
            {
                handle_flag_(*it);
            }
            else
            {
                break;
            }
        }
        // 没遇到%，说明都是用户字符
        else
        {
            if (!user_chars)
            {
                user_chars = std::make_unique<aggregate_formatter>();
            }
            user_chars->add_char(*it);
        }
    }

    if (user_chars)
    {
        formatters_.emplace_back(std::move(user_chars));
    }
}

} // namespace mylog

