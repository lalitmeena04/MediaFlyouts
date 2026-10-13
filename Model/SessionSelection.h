#pragma once

#include <QString>
#include <QStringList>

namespace mf {

int normalizedSessionCount(int sessionCount);

// WinRT calls can return different proxy objects for same session.
int selectedSessionIndexByAppId(const QStringList& appIds, const QString& selectedAppId);

} // namespace mf
