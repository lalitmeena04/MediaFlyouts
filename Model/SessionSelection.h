#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace mf {

int normalizedSessionCount(int sessionCount);

// WinRT calls can return different proxy objects for same session.
int selectedSessionIndexByAppId(const QStringList& appIds, const QString& selectedAppId);

// Browsers expose one SMTC session per app. After a pause the browser can
// promote the next playing tab under the same app id. Sweep targets are
// sessions that still play for an app id we already paused.
QList<int> pauseSweepTargets(const QStringList& appIds, const QList<bool>& playing, const QStringList& sweptAppIds);

} // namespace mf
