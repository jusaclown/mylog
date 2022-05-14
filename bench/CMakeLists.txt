add_executable(bench bench.cc)
mylog_enable_warnings(bench)
target_link_libraries(bench PRIVATE mylog::mylog)