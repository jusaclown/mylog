#include "log/details/file_helper.h"
#include "log/details/os.h"
#include "log/common.h"

namespace mylog {
namespace details {


file_helper::~file_helper()
{
    close();
}

void file_helper::open(filename_t filename, bool truncate)
{
    close();
    filename_ = std::move(filename);
    
    const char* append_mode = "ab";
    const char* truncate_mode = "wb";

    for (int i = 0; i < open_tries_; ++i)
    {
        os::create_dir(os::dirname(filename_));
        if (truncate)
        {
            std::FILE* tmp;
            if ((tmp = std::fopen(filename_.c_str(), truncate_mode)) == nullptr)
            {
                continue;
            }
            std::fclose(tmp);
        }
        if ((fp_ = std::fopen(filename_.c_str(), append_mode)) != nullptr)
        {
            return;
        }
        os::sleep_for_millis(open_iterval_);
    }

    throw_mylog_ex("Failed opening file " + os::filename_to_str(filename_) + " for writing", errno);
}

void file_helper::reopen(bool truncate)
{
    if (filename_.empty())
    {
        throw_mylog_ex("Failed re opening file - was not opened before");
    }
    this->open(filename_, truncate);
}

void file_helper::flush()
{
    if (std::fflush(fp_) != 0)
    {
        throw_mylog_ex("Failed flush to file " + os::filename_to_str(filename_), errno);
    }
}

void file_helper::close()
{
    if (fp_ != nullptr)
    {
        std::fclose(fp_);
        fp_ = nullptr;
    }
}

void file_helper::write(const memory_buf_t& buf)
{
    auto msg_size = buf.size();
    if (std::fwrite(buf.data(), sizeof(char), msg_size, fp_) != msg_size)
    {
        throw_mylog_ex("Failed writing to file " + os::filename_to_str(filename_), errno);
    }
}

std::size_t file_helper::size() const
{
    if (fp_ == nullptr)
    {
        throw_mylog_ex("Cannot use size() on closed file " + os::filename_to_str(filename_));
    }
    return os::filesize(fp_);
}

const filename_t& file_helper::filename() const
{
    return filename_;
}

std::tuple<filename_t, filename_t> file_helper::split_by_extension(const filename_t& filename)
{
    auto ext_index  = filename.find_last_of('.');
    if (ext_index  == filename_t::npos || ext_index  == filename.size() - 1 || ext_index  == 0)
    {
        return std::make_tuple(filename, "");
    }

    auto sep = filename.find_last_of('/');
    if (sep != filename_t::npos && sep >= ext_index  - 1)
    {
        return std::make_tuple(filename, "");
    }

    return std::make_tuple(filename.substr(0, ext_index), filename.substr(ext_index));
}

    
} // namespace details
} // namespace mylog