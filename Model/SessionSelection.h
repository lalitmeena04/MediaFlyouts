#pragma once

#include <QString>
#include <QStringList>

namespace mf {

// Clamp session count to at least 1. Reserve empty-state UI slot.
int normalizedSessionCount(int sessionCount);

// Get first matching index by app id. Return 0 if not found.
// WinRT calls can return different proxy objects for same session.
int selectedSessionIndexByAppId(const QStringList& appIds, const QString& selectedAppId);

} // namespace mf
