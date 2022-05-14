#pragma once

#include "log/common.h"

#include <cstdio>
#include <tuple>

namespace mylog {
namespace details {

class file_helper
{
public:
    file_helper() = default;
    ~file_helper();

    file_helper(const file_helper&) = delete;
    file_helper& operator=(const file_helper&) = delete;

    void open(filename_t filename, bool truncate = false);
    void reopen(bool truncate = false);
    void flush();
    void close();
    void write(const memory_buf_t& buf);
    std::size_t size() const;
    const filename_t& filename() const;

    //
    // return file path and its extension:
    //
    // "mylog.txt" => ("mylog", ".txt")
    // "mylog" => ("mylog", "")
    // "mylog." => ("mylog.", "")
    // "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
    //
    // the starting dot in filenames is ignored (hidden files):
    //
    // ".mylog" => (".mylog". "")
    // "my_folder/.mylog" => ("my_folder/.mylog", "")
    // "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
    static std::tuple<filename_t, filename_t> split_by_extension(const filename_t& filename);

private:
    const int open_tries_ = 5;
    const unsigned int open_iterval_ = 10;
    filename_t filename_;
    std::FILE* fp_{ nullptr };
};

    
} // namespace details
} // namespace mylog
