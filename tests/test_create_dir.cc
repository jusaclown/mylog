/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/mylog/master/LICENSE
 */
#include "includes.h"

using mylog::details::os::create_dir;
using mylog::details::os::path_exists;

bool try_create_dir(const mylog::filename_t &path, const mylog::filename_t &normalized_path)
{
    auto rv = create_dir(path);
    REQUIRE(rv == true);
    return path_exists(normalized_path);
}

TEST_CASE("create_dir", "[create_dir]")
{
    prepare_logdir();

    REQUIRE(try_create_dir(MYLOG_FILENAME_T("test_logs/dir1/dir1"), MYLOG_FILENAME_T("test_logs/dir1/dir1")));
    REQUIRE(try_create_dir(MYLOG_FILENAME_T("test_logs/dir1/dir1"), MYLOG_FILENAME_T("test_logs/dir1/dir1"))); // test existing
    REQUIRE(try_create_dir(MYLOG_FILENAME_T("test_logs/dir1///dir2//"), MYLOG_FILENAME_T("test_logs/dir1/dir2")));
    REQUIRE(try_create_dir(MYLOG_FILENAME_T("./test_logs/dir1/dir3"), MYLOG_FILENAME_T("test_logs/dir1/dir3")));
    REQUIRE(try_create_dir(MYLOG_FILENAME_T("test_logs/../test_logs/dir1/dir4"), MYLOG_FILENAME_T("test_logs/dir1/dir4")));
}

TEST_CASE("create_invalid_dir", "[create_dir]")
{
    REQUIRE(create_dir(MYLOG_FILENAME_T("")) == false);
    REQUIRE(create_dir(mylog::filename_t{}) == false);
    REQUIRE(create_dir("/proc/mylog-utest") == false);
}

TEST_CASE("dirname", "[create_dir]")
{
    using mylog::details::os::dirname;
    REQUIRE(dirname(MYLOG_FILENAME_T("")).empty());
    REQUIRE(dirname(MYLOG_FILENAME_T("dir")).empty());
    REQUIRE(dirname(MYLOG_FILENAME_T("dir/")) == MYLOG_FILENAME_T("dir"));
    REQUIRE(dirname(MYLOG_FILENAME_T("dir///")) == MYLOG_FILENAME_T("dir//"));
    REQUIRE(dirname(MYLOG_FILENAME_T("dir/file")) == MYLOG_FILENAME_T("dir"));
    REQUIRE(dirname(MYLOG_FILENAME_T("dir/file.txt")) == MYLOG_FILENAME_T("dir"));
    REQUIRE(dirname(MYLOG_FILENAME_T("dir/file.txt/")) == MYLOG_FILENAME_T("dir/file.txt"));
    REQUIRE(dirname(MYLOG_FILENAME_T("/dir/file.txt")) == MYLOG_FILENAME_T("/dir"));
    REQUIRE(dirname(MYLOG_FILENAME_T("//dir/file.txt")) == MYLOG_FILENAME_T("//dir"));
    REQUIRE(dirname(MYLOG_FILENAME_T("../file.txt")) == MYLOG_FILENAME_T(".."));
    REQUIRE(dirname(MYLOG_FILENAME_T("./file.txt")) == MYLOG_FILENAME_T("."));
}
