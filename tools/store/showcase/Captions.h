#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

// Poster texts from captions.json: language -> poster name -> headline, subtitle.
// The Store listing script reads the same file for the screenshot captions.
namespace mf::showcase {

// Empty object and a filled error when the file cannot be read or parsed.
QJsonObject loadCaptions(const QString& path, QString* error);

// One line per gap. Every language must have the posters of the first
// language, each with a non-empty headline and subtitle.
QStringList captionProblems(const QJsonObject& captions, const QStringList& languages);

} // namespace mf::showcase
