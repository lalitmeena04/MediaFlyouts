#pragma once

#include <QString>
#include <QStringList>

namespace mf {

// clamps a live session count to at least 1, so "no sessions" still reserves a slot for
// the empty-state UI instead of collapsing to zero
int normalizedSessionCount(int sessionCount);

// index of the first entry in appIds matching selectedAppId (a session's stable
// SourceAppUserModelId); 0 if selectedAppId is empty or not found. Matching by app id
// rather than WinRT object identity, since separate SMTC calls can hand back distinct
// proxy objects for the same underlying session.
int selectedSessionIndexByAppId(const QStringList& appIds, const QString& selectedAppId);

} // namespace mf
