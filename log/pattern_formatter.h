#pragma once

#include "log/formatter.h"
#include "log/details/log_msg.h"

#include <vector>

namespace mylog {

class flag_formatter
{
public:
    virtual ~flag_formatter() = default;
    virtual void format(const details::log_msg&msg, const std::tm& tm_time, memory_buf_t& dest) = 0;
};


class pattern_formatter : public formatter
{
public:
    pattern_formatter();
    explicit pattern_formatter(std::string pattern);

    pattern_formatter(const pattern_formatter &other) = delete;
    pattern_formatter &operator=(const pattern_formatter &other) = delete;

    void format(const details::log_msg& msg, memory_buf_t& dest) override;
    std::unique_ptr<formatter> clone() const override;

    void set_pattern(std::string pattern);

private:
    // 用于将pattern解析成对应的flag_formatter 
    void compile_pattern_();
    void handle_flag_(const char ch);


private:
    std::string pattern_;
    std::vector<std::unique_ptr<flag_formatter>> formatters_;
    std::chrono::seconds last_secs_{ 0 };
    std::tm cached_tm_;
};

} // namespace mylog
