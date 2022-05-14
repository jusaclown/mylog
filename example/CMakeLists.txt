add_executable(example example.cc)
mylog_enable_warnings(example)
target_link_libraries(example PRIVATE mylog::mylog)

add_executable(string_test string_test.cc)

find_package(fmt REQUIRED)
add_executable(fmt_test fmt_test.cc)
target_link_libraries(fmt_test PRIVATE fmt::fmt)