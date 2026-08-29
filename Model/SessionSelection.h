#pragma once

#include <QVector>

namespace mf {

// Resolves widget-indicator state from the current media-session list. The selected-session
// match vector is built by the Windows provider, while this policy stays platform independent.
int normalizedSessionCount(int sessionCount);
int selectedSessionIndex(const QVector<bool>& selectedMatches);

} // namespace mf
