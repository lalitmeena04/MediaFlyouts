#include "StartupController.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.h>

#include "Logging.h"

using winrt::Windows::ApplicationModel::StartupTask;
using winrt::Windows::ApplicationModel::StartupTaskState;

namespace {
constexpr wchar_t kStartupTaskId[] = L"MediaFlyoutsStartupTask";
}

StartupController::StartupController(QObject* parent) : QObject(parent), m_appName(QStringLiteral("MediaFlyouts")) {}

QString StartupController::runKeyPath() {
    return QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
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
        // Remove old Run key to prevent double launch.
        QSettings(runKeyPath(), QSettings::NativeFormat).remove(m_appName);
        TRC_INFO("startup: StartupTask enabled={} state={}", enable, static_cast<int>(task.State()));
        return;
    } catch(const winrt::hresult_error& e) {
        // Expected for unpackaged installs. Registry Run key takes over.
        TRC_INFO("startup: StartupTask unavailable (hr={:#x}), using Run key", static_cast<unsigned long>(e.code()));
    }

    QSettings run(runKeyPath(), QSettings::NativeFormat);
    if(enable) {
        const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        run.setValue(m_appName, QStringLiteral("\"%1\"").arg(exe));
    } else {
        run.remove(m_appName);
    }
    run.sync();
    if(run.status() != QSettings::NoError)
        TRC_WARN("startup: Run key write failed (status={})", static_cast<int>(run.status()));
    else
        TRC_INFO("startup: Run key enabled={}", enable);
}
