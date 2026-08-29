#include "SessionSelection.h"

namespace mf {

int normalizedSessionCount(int sessionCount) {
    return sessionCount < 1 ? 1 : sessionCount;
}

int selectedSessionIndex(const QVector<bool>& selectedMatches) {
    for(int index = 0; index < selectedMatches.size(); ++index)
        if(selectedMatches.at(index))
            return index;
    return 0;
}

} // namespace mf
