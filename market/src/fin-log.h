#pragma once
#define FMT_UNICODE 0
#include "spdlog/spdlog.h"

#define LOG_INFO(x, ...) spdlog::info(x, ##__VA_ARGS__)
#define LOG_WARN(x, ...) spdlog::warn(x, ##__VA_ARGS__)
