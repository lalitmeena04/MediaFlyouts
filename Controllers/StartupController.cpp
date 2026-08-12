#include "StartupController.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

StartupController::StartupController(QObject* parent) : QObject(parent), m_appName(QStringLiteral("MediaFlyouts")) {}

QString StartupController::runKeyPath() {
    return QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
}

bool StartupController::isEnabled() const {
    QSettings run(runKeyPath(), QSettings::NativeFormat);
    return run.contains(m_appName);
}

void StartupController::setEnabled(bool enable) {
    QSettings run(runKeyPath(), QSettings::NativeFormat);
    if(enable) {
        const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        run.setValue(m_appName, QStringLiteral("\"%1\"").arg(exe));
    } else {
        run.remove(m_appName);
    }
}
