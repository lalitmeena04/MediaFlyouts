#include "StartupController.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.h>

using winrt::Windows::ApplicationModel::StartupTask;
using winrt::Windows::ApplicationModel::StartupTaskState;

namespace {
constexpr wchar_t kStartupTaskId[] = L"MediaFlyoutsStartupTask";
}

StartupController::StartupController(QObject* parent) : QObject(parent), m_appName(QStringLiteral("MediaFlyouts")) {}

QString StartupController::runKeyPath() {
    return QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
}

bool StartupController::isEnabled() const {
    try {
        const auto task = StartupTask::GetAsync(kStartupTaskId).get();
        const auto state = task.State();
        return state == StartupTaskState::Enabled || state == StartupTaskState::EnabledByPolicy;
    } catch(const winrt::hresult_error&) {}
    QSettings run(runKeyPath(), QSettings::NativeFormat);
    return run.contains(m_appName);
}

void StartupController::setEnabled(bool enable) {
    try {
        auto task = StartupTask::GetAsync(kStartupTaskId).get();
        if(enable) {
            if(task.State() == StartupTaskState::Disabled)
                task.RequestEnableAsync().get();
        } else {
            task.Disable();
        }
        // StartupTask is source of truth for packaged builds.
        // Remove old Run key to prevent double launch.
        QSettings(runKeyPath(), QSettings::NativeFormat).remove(m_appName);
        return;
    } catch(const winrt::hresult_error&) {}

    QSettings run(runKeyPath(), QSettings::NativeFormat);
    if(enable) {
        const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        run.setValue(m_appName, QStringLiteral("\"%1\"").arg(exe));
    } else {
        run.remove(m_appName);
    }
}
