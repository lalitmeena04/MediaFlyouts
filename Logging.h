#pragma once

// SPDLOG_ACTIVE_LEVEL is set to DEBUG by the build so TRC_DEBUG is compiled in.
#include <spdlog/spdlog.h>

#include <QString>

// Custom trace macros mapped onto spdlog's source-location-aware macros. Using these keeps a
// single logging vocabulary across the codebase and lets the active level be tuned centrally.
#define TRC_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define TRC_INFO(...)  SPDLOG_INFO(__VA_ARGS__)
#define TRC_WARN(...)  SPDLOG_WARN(__VA_ARGS__)
#define TRC_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

namespace mf {
// Installs a rotating-file (+ debugger) logger as the default spdlog logger. Call once at startup.
// Returns the absolute path of the active log file.
QString initLogging();
}
