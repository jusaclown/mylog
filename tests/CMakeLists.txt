include(../cmake/utils.cmake)

set(MYLOG_UTESTS_SOURCES
    main.cc
    utils.cc
    test_formatter.cc
    test_file_helper.cc
    test_create_dir.cc
    test_fmt_helper.cc
    test_file_logging.cc
    test_daily_logger.cc
    test_mpmc_q.cc
    test_async.cc
    )

add_executable(mylog-utests ${MYLOG_UTESTS_SOURCES})
target_link_libraries(mylog-utests PRIVATE mylog::mylog)

if (MYLOG_BUILD_WARNINGS)
    mylog_enable_warnings(mylog-utests)
endif()

if (MYLOG_SANITIZE_ADDRESS)
    mylog_enable_sanitizer(mylog-utests)
endif()

add_test(NAME mylog-utests COMMAND mylog-utests)
set_tests_properties(mylog-utests PROPERTIES RUN_SERIAL ON)