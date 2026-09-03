#include "QmlLogger.h"

#include "Logging.h"

QmlLogger::QmlLogger(QObject* parent) : QObject(parent) {}

void QmlLogger::debug(const QString& message) const {
    TRC_DEBUG("[qml] {}", message.toStdString());
}

void QmlLogger::info(const QString& message) const {
    TRC_INFO("[qml] {}", message.toStdString());
}

void QmlLogger::warn(const QString& message) const {
    TRC_WARN("[qml] {}", message.toStdString());
}

void QmlLogger::error(const QString& message) const {
    TRC_ERROR("[qml] {}", message.toStdString());
}
