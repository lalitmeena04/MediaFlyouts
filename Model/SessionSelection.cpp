#include "SessionSelection.h"

namespace mf {

int normalizedSessionCount(int sessionCount) {
    return sessionCount < 1 ? 1 : sessionCount;
}

int selectedSessionIndexByAppId(const QStringList& appIds, const QString& selectedAppId) {
    if(selectedAppId.isEmpty())
        return 0;
    for(int i = 0; i < appIds.size(); ++i)
        if(appIds.at(i) == selectedAppId)
            return i;
    return 0;
}

QList<int> pauseSweepTargets(const QStringList& appIds, const QList<bool>& playing, const QStringList& sweptAppIds) {
    QList<int> targets;
    const int n = qMin(appIds.size(), playing.size());
    for(int i = 0; i < n; ++i) {
        if(!playing.at(i))
            continue;
        if(appIds.at(i).isEmpty())
            continue;
        if(sweptAppIds.contains(appIds.at(i)))
            targets.append(i);
    }
    return targets;
}

} // namespace mf
