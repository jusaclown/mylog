/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/mylog/master/LICENSE
 */
#include "includes.h"

#define TEST_FILENAME "test_logs/file_helper_test.txt"

using mylog::details::file_helper;

static void write_with_helper(file_helper &helper, size_t howmany)
{
    mylog::memory_buf_t formatted;
    fmt::format_to(std::back_inserter(formatted), "{}", std::string(howmany, '1'));
    helper.write(formatted);
    helper.flush();
}

TEST_CASE("file_helper_filename", "[file_helper::filename()]]")
{
    prepare_logdir();

    file_helper helper;
    mylog::filename_t target_filename = TEST_FILENAME;
    helper.open(target_filename);
    REQUIRE(helper.filename() == target_filename);
}

TEST_CASE("file_helper_size", "[file_helper::size()]]")
{
    prepare_logdir();
    mylog::filename_t target_filename = TEST_FILENAME;
    size_t expected_size = 123;
    {
        file_helper helper;
        helper.open(target_filename);
        write_with_helper(helper, expected_size);
        REQUIRE(static_cast<size_t>(helper.size()) == expected_size);
    }
    REQUIRE(get_filesize(TEST_FILENAME) == expected_size);
}

TEST_CASE("file_helper_reopen", "[file_helper::reopen()]]")
{
    prepare_logdir();
    mylog::filename_t target_filename = TEST_FILENAME;
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, 12);
    REQUIRE(helper.size() == 12);
    helper.reopen(true);
    REQUIRE(helper.size() == 0);
}

TEST_CASE("file_helper_reopen2", "[file_helper::reopen(false)]]")
{
    prepare_logdir();
    mylog::filename_t target_filename = TEST_FILENAME;
    size_t expected_size = 14;
    file_helper helper;
    helper.open(target_filename);
    write_with_helper(helper, expected_size);
    REQUIRE(helper.size() == expected_size);
    helper.reopen(false);
    REQUIRE(helper.size() == expected_size);
}

static void test_split_ext(const mylog::filename_t::value_type *fname, const mylog::filename_t::value_type *expect_base,
    const mylog::filename_t::value_type *expect_ext)
{
    mylog::filename_t filename(fname);
    mylog::filename_t expected_base(expect_base);
    mylog::filename_t expected_ext(expect_ext);

    mylog::filename_t basename;
    mylog::filename_t ext;
    std::tie(basename, ext) = file_helper::split_by_extension(filename);
    REQUIRE(basename == expected_base);
    REQUIRE(ext == expected_ext);
}

TEST_CASE("file_helper_split_by_extension", "[file_helper::split_by_extension()]]")
{
    test_split_ext("mylog.txt", "mylog", ".txt");
    test_split_ext(".mylog.txt", ".mylog", ".txt");
    test_split_ext(".mylog", ".mylog", "");
    test_split_ext("/aaa/bb.d/mylog", "/aaa/bb.d/mylog", "");
    test_split_ext("/aaa/bb.d/mylog.txt", "/aaa/bb.d/mylog", ".txt");
    test_split_ext("aaa/bbb/ccc/mylog.txt", "aaa/bbb/ccc/mylog", ".txt");
    test_split_ext("aaa/bbb/ccc/mylog.", "aaa/bbb/ccc/mylog.", "");
    test_split_ext("aaa/bbb/ccc/.mylog.txt", "aaa/bbb/ccc/.mylog", ".txt");
    test_split_ext("/aaa/bbb/ccc/mylog.txt", "/aaa/bbb/ccc/mylog", ".txt");
    test_split_ext("/aaa/bbb/ccc/.mylog", "/aaa/bbb/ccc/.mylog", "");
    test_split_ext("../mylog.txt", "../mylog", ".txt");
    test_split_ext(".././mylog.txt", ".././mylog", ".txt");
    test_split_ext(".././mylog.txt/xxx", ".././mylog.txt/xxx", "");
    test_split_ext("/mylog.txt", "/mylog", ".txt");
    test_split_ext("//mylog.txt", "//mylog", ".txt");
    test_split_ext("", "", "");
    test_split_ext(".", ".", "");
    test_split_ext("..txt", ".", ".txt");
}

TEST_CASE("file_helper_open", "[file_helper]")
{
    prepare_logdir();
    mylog::filename_t target_filename = TEST_FILENAME;
    file_helper helper;
    helper.open(target_filename);
    helper.close();

    target_filename += "/invalid";
    REQUIRE_THROWS_AS(helper.open(target_filename), mylog::log_ex);
}
