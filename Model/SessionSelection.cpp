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

} // namespace mf
