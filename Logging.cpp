#include "Logging.h"

#include <memory>
#include <vector>

#include <QDir>
#include <QStandardPaths>

#include <spdlog/sinks/rotating_file_sink.h>
#ifdef _WIN32
#include <spdlog/sinks/msvc_sink.h>
#endif

namespace mf {

QString initLogging() {
#if SPDLOG_ACTIVE_LEVEL >= SPDLOG_LEVEL_OFF
    // Logging disabled at build time. No file. No sinks.
    return {};
#else
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/logs");
    QDir().mkpath(dir);
    const QString file = dir + QStringLiteral("/mediaflyouts.log");

    try {
        std::vector<spdlog::sink_ptr> sinks;
        // 5 MB per file, keep 3 rotated files
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file.toStdString(), 5 * 1024 * 1024, 3));
#ifdef _WIN32
        sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif
        auto logger = std::make_shared<spdlog::logger>("mediaflyouts", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);
        logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%L] [t:%t] %v  (%s:%#)");
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::debug);
    } catch(const std::exception&) {
        // Keep process alive if file sink fails.
        // Default stdout logger stays active.
        spdlog::set_level(spdlog::level::debug);
    }
    return file;
#endif
}

} // namespace mf
