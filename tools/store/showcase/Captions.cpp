#include "Captions.h"

#include <QFile>
#include <QJsonDocument>

namespace mf::showcase {

QJsonObject loadCaptions(const QString& path, QString* error) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        if(error)
            *error = QStringLiteral("cannot open %1").arg(path);
        return {};
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if(!document.isObject()) {
        if(error)
            *error = QStringLiteral("%1: %2").arg(path, parseError.errorString());
        return {};
    }
    return document.object();
}

QStringList captionProblems(const QJsonObject& captions, const QStringList& languages) {
    QStringList problems;
    if(languages.isEmpty())
        return {QStringLiteral("no languages")};
    const QStringList posters = captions.value(languages.first()).toObject().keys();
    if(posters.isEmpty())
        problems << QStringLiteral("%1: no posters").arg(languages.first());

    for(const QString& language : languages) {
        const QJsonObject texts = captions.value(language).toObject();
        for(const QString& poster : posters) {
            const QJsonObject entry = texts.value(poster).toObject();
            for(const QString& key : {QStringLiteral("headline"), QStringLiteral("subtitle")})
                if(entry.value(key).toString().trimmed().isEmpty())
                    problems << QStringLiteral("%1: %2 has no %3").arg(language, poster, key);
        }
        for(const QString& poster : texts.keys())
            if(!posters.contains(poster))
                problems << QStringLiteral("%1: unknown poster %2").arg(language, poster);
    }
    return problems;
}

} // namespace mf::showcase
