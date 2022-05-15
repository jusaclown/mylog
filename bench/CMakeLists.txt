add_executable(bench bench.cc)
mylog_enable_warnings(bench)
target_link_libraries(bench PRIVATE mylog::mylog)

add_executable(async_bench async_bench.cc)
mylog_enable_warnings(async_bench)
target_link_libraries(async_bench PRIVATE mylog::mylog)