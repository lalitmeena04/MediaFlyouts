#pragma once

#include <QImage>

// Generated demo artwork. Store screenshots must not carry third-party
// album covers or app icons.
namespace mf::demo {

int coverCount();
// Square cover. The index wraps.
QImage coverArt(int index, int size = 640);

int appIconCount();
// Rounded tile with a generic glyph. The index wraps.
QImage appIcon(int index, int size = 96);

} // namespace mf::demo
