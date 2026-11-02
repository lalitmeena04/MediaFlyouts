#pragma once

#include <QString>

namespace mf {

// "1.7.0-263.abc" -> "1.7.0". Leading "v" is dropped.
QString baseVersion(const QString& version);

// True only when a different version ran before. A first run records, it does not show.
bool shouldShowWhatsNew(const QString& lastSeenVersion, const QString& currentVersion);

// Bundled WHATS_NEW.md. Empty when the resource is missing.
QString whatsNewText();

// Version named on the first line ("Version 1.7.0"). Empty when absent.
QString whatsNewVersion(const QString& text);

// Text without the "Version ..." line. The dialog title already carries the version.
QString whatsNewBody(const QString& text);

} // namespace mf
