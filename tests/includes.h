#pragma once

#include "catch.hpp"
#include "utils.h"
#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <memory>
#include <fmt/core.h>
#include <fmt/format.h>

#define MYLOG_ACTIVE_LEVEL MYLOG_LEVEL_DEBUG

#include "log/pattern_formatter.h"
#include "log/common.h"
#include "log/details/file_helper.h"
#include "log/mylog.h"
#include "log/details/os.h"
#include "log/sinks/basic_file_sink.h"
#include "log/sinks/rotating_file_sink.h"
#include "log/sinks/daily_file_sink.h"
#include "log/details/mpmc_blocking_queue.h"

#define MYLOG_FILENAME_T(t) t