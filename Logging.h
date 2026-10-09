#pragma once

// Build sets SPDLOG_ACTIVE_LEVEL. Debug builds compile TRC_DEBUG in. Release builds start at INFO.
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
// Also routes Qt and QML messages into the same log.
// Returns the absolute path of the active log file.
QString initLogging();
// Folder that holds the rotating log files. Empty when logging is off.
QString logDir();
// Write buffered lines to disk now.
void flushLogging();
// Size cap per log file and number of rotated files kept.
constexpr qint64 kLogFileMaxBytes = 2 * 1024 * 1024;
constexpr int kLogFilesKept = 3;
}
