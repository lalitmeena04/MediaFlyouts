#pragma once

// Build sets SPDLOG_ACTIVE_LEVEL to DEBUG. TRC_DEBUG is compiled in.
#include <spdlog/spdlog.h>

#include <QString>

// Custom trace macros map to spdlog source-location macros.
// One logging vocabulary is used across codebase.
#define TRC_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define TRC_INFO(...)  SPDLOG_INFO(__VA_ARGS__)
#define TRC_WARN(...)  SPDLOG_WARN(__VA_ARGS__)
#define TRC_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

namespace mf {
// Install rotating-file and debugger logger. Call once at startup.
// Returns the absolute path of the active log file.
QString initLogging();
}
