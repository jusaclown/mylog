#include <fmt/format.h>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <ctime>
#include <fmt/chrono.h>

// generate fmt datetime format string, e.g. {:%Y-%m-%d}.
static std::string calc_filename(const std::string& filename, const tm& now_tm)
{
    std::string fmt_filename = fmt::format("{{:{}}}", filename);
    return fmt::format(fmt::runtime(fmt_filename), now_tm);
    return fmt_filename;
}

int main()
{
    time_t tnow = time(nullptr);
    std::tm tm;
    localtime_r(&tnow, &tm);
    
    std::cout << tm.tm_hour << std::endl;
    std::cout << calc_filename("myapp-%Y-%m-%d:%H:%M:%S.log", tm) << std::endl;
        
}