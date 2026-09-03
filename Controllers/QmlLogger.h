#pragma once

#include <QObject>
#include <QString>

// Thin bridge so QML can write into the same spdlog file logger the C++ side uses.
// Exposed to QML as the context property "log".
class QmlLogger : public QObject {
        Q_OBJECT

    public:
        explicit QmlLogger(QObject* parent = nullptr);

        Q_INVOKABLE void debug(const QString& message) const;
        Q_INVOKABLE void info(const QString& message) const;
        Q_INVOKABLE void warn(const QString& message) const;
        Q_INVOKABLE void error(const QString& message) const;
};
