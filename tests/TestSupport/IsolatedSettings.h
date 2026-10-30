#pragma once

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>

namespace mftest {

// Points user-scope ini settings at a scratch folder for this process.
// QStandardPaths test mode does not move QSettings ini files on Windows, so
// without this the tests would wipe the real %APPDATA%\MediaFlyouts store.
// Call once before the first Settings or QSettings object is created.
inline QString isolateUserSettings() {
    static QTemporaryDir scratch;
    static bool applied = false;
    if(!applied) {
        QStandardPaths::setTestModeEnabled(true);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, scratch.path());
        applied = true;
    }
    return QDir::cleanPath(scratch.path());
}

// Clears the isolated user store. Safe to call from every test setup.
inline void clearUserSettings() {
    isolateUserSettings();
    QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
              QStringLiteral("MediaFlyouts"))
        .clear();
}

} // namespace mftest
