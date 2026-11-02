#include <gtest/gtest.h>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QXmlStreamReader>

#include "Localization.h"

// Guards the shipped .ts files. Every language in Localization.cpp needs a complete
// file, and every translation must keep the %1/%2 placeholders of its source.
namespace {

QString tsPath(const QString& code) {
    return QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/i18n/") + mf::translationFileName(code) + QStringLiteral(".ts");
}

struct Message {
        QString context;
        QString source;
        QString translation;
        bool unfinished = false;
};

struct TsFile {
        QString language;
        QString sourceLanguage;
        QList<Message> messages;
        QString error;
};

TsFile parseTs(const QString& path) {
    TsFile out;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        out.error = QStringLiteral("cannot open ") + path;
        return out;
    }
    QXmlStreamReader xml(&file);
    QString context;
    Message current;
    while(!xml.atEnd()) {
        xml.readNext();
        if(xml.isStartElement()) {
            const QString name = xml.name().toString();
            if(name == QLatin1String("TS")) {
                out.language = xml.attributes().value(QStringLiteral("language")).toString();
                out.sourceLanguage = xml.attributes().value(QStringLiteral("sourcelanguage")).toString();
            } else if(name == QLatin1String("name")) {
                context = xml.readElementText();
            } else if(name == QLatin1String("message")) {
                current = Message{};
                current.context = context;
            } else if(name == QLatin1String("source")) {
                current.source = xml.readElementText();
            } else if(name == QLatin1String("translation")) {
                current.unfinished = xml.attributes().value(QStringLiteral("type")) == QLatin1String("unfinished");
                current.translation = xml.readElementText();
            }
        } else if(xml.isEndElement() && xml.name() == QLatin1String("message")) {
            out.messages << current;
        }
    }
    if(xml.hasError())
        out.error = xml.errorString();
    return out;
}

QStringList placeholders(const QString& text) {
    static const QRegularExpression re(QStringLiteral("%\\d"));
    QStringList found;
    auto it = re.globalMatch(text);
    while(it.hasNext())
        found << it.next().captured(0);
    found.sort();
    return found;
}

QStringList shippedCodes() {
    QStringList codes;
    for(const mf::LanguageInfo& info : mf::supportedLanguages())
        if(info.code != mf::defaultLanguage())
            codes << info.code;
    return codes;
}

QString readSource(const QString& relativePath) {
    QFile file(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/") + relativePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(file.readAll());
}

// Turns the escaped body of a C++ or JS string literal into the text lupdate stores.
QString unescapeLiteral(const QString& body) {
    QString out;
    out.reserve(body.size());
    for(int i = 0; i < body.size(); ++i) {
        const QChar c = body.at(i);
        if(c != QLatin1Char('\\') || i + 1 >= body.size()) {
            out += c;
            continue;
        }
        const QChar next = body.at(++i);
        if(next == QLatin1Char('n'))
            out += QLatin1Char('\n');
        else if(next == QLatin1Char('t'))
            out += QLatin1Char('\t');
        else if(next == QLatin1Char('u') && i + 4 < body.size()) {
            out += QChar(body.mid(i + 1, 4).toUShort(nullptr, 16));
            i += 4;
        } else
            out += next; // \" \\ \'
    }
    return out;
}

// Every string literal that directly follows `prefixPattern`, e.g. `qsTr\(`, unescaped.
QSet<QString> literalsOf(const QString& source, const QString& prefixPattern) {
    const QRegularExpression re(prefixPattern + QStringLiteral("\\s*\"((?:[^\"\\\\]|\\\\.)*)\"\\s*\\)"));
    QSet<QString> found;
    auto it = re.globalMatch(source);
    while(it.hasNext())
        found.insert(unescapeLiteral(it.next().captured(1)));
    return found;
}

QStringList sorted(const QSet<QString>& set) {
    QStringList list = set.values();
    list.sort();
    return list;
}

// Readable set comparison. gtest prints QStringList as raw bytes.
void expectSameStrings(const QSet<QString>& inSources, const QSet<QString>& inTs, const QString& where) {
    const QStringList missingInTs = sorted(inSources - inTs);
    const QStringList staleInTs = sorted(inTs - inSources);
    EXPECT_TRUE(missingInTs.isEmpty() && staleInTs.isEmpty())
        << where.toStdString() << ": run `cmake --build build\\msvc-debug --target update_translations`\n"
        << "  in sources but not in .ts: " << missingInTs.join(QStringLiteral(" | ")).toStdString() << "\n"
        << "  in .ts but not in sources: " << staleInTs.join(QStringLiteral(" | ")).toStdString();
}

// Context -> source strings, taken from the first shipped .ts file. Every file
// carries the same set (checked by EveryLanguageCoversTheSameSourceStrings).
QMap<QString, QSet<QString>> tsSourcesByContext() {
    QMap<QString, QSet<QString>> out;
    const TsFile ts = parseTs(tsPath(shippedCodes().first()));
    for(const Message& m : ts.messages)
        out[m.context].insert(m.source);
    return out;
}

} // namespace

TEST(TranslationFilesTest, EveryShippedLanguageHasATsFileAndNoExtraFilesExist) {
    QSet<QString> expected;
    for(const QString& code : shippedCodes()) {
        expected.insert(mf::translationFileName(code) + QStringLiteral(".ts"));
        EXPECT_TRUE(QFile::exists(tsPath(code))) << tsPath(code).toStdString();
    }
    const QDir dir(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/i18n"));
    const QStringList files = dir.entryList({QStringLiteral("*.ts")}, QDir::Files);
    for(const QString& file : files)
        EXPECT_TRUE(expected.contains(file)) << "Unlisted translation file: " << file.toStdString();
}

TEST(TranslationFilesTest, TsHeaderNamesTheLanguageAndEnglishSource) {
    for(const QString& code : shippedCodes()) {
        const TsFile ts = parseTs(tsPath(code));
        ASSERT_TRUE(ts.error.isEmpty()) << ts.error.toStdString();
        EXPECT_EQ(ts.language, code);
        EXPECT_EQ(ts.sourceLanguage, QStringLiteral("en"));
    }
}

TEST(TranslationFilesTest, EveryMessageIsTranslated) {
    for(const QString& code : shippedCodes()) {
        const TsFile ts = parseTs(tsPath(code));
        ASSERT_TRUE(ts.error.isEmpty()) << ts.error.toStdString();
        EXPECT_GT(ts.messages.size(), 100) << code.toStdString();
        for(const Message& m : ts.messages) {
            EXPECT_FALSE(m.unfinished) << code.toStdString() << " [" << m.context.toStdString()
                                       << "] unfinished: " << m.source.toStdString();
            EXPECT_FALSE(m.translation.trimmed().isEmpty())
                << code.toStdString() << " [" << m.context.toStdString() << "] empty: " << m.source.toStdString();
        }
    }
}

TEST(TranslationFilesTest, TranslationsKeepTheSourcePlaceholders) {
    for(const QString& code : shippedCodes()) {
        const TsFile ts = parseTs(tsPath(code));
        ASSERT_TRUE(ts.error.isEmpty()) << ts.error.toStdString();
        for(const Message& m : ts.messages)
            EXPECT_EQ(placeholders(m.source), placeholders(m.translation))
                << code.toStdString() << ": " << m.source.toStdString() << " -> " << m.translation.toStdString();
    }
}

TEST(TranslationFilesTest, EveryLanguageCoversTheSameSourceStrings) {
    // All files come from one lupdate run, so their source sets must match.
    QSet<QString> reference;
    QString referenceCode;
    for(const QString& code : shippedCodes()) {
        const TsFile ts = parseTs(tsPath(code));
        ASSERT_TRUE(ts.error.isEmpty()) << ts.error.toStdString();
        QSet<QString> keys;
        for(const Message& m : ts.messages)
            keys.insert(m.context + QLatin1Char('\x1f') + m.source);
        if(referenceCode.isEmpty()) {
            reference = keys;
            referenceCode = code;
            continue;
        }
        EXPECT_EQ(keys, reference) << code.toStdString() << " differs from " << referenceCode.toStdString();
    }
}

TEST(TranslationFilesTest, CMakeLanguageListMatchesTheLocalizationModel) {
    // The list is declared twice: once for lrelease, once for the picker. Keep them equal.
    const QString cmake = readSource(QStringLiteral("CMakeLists.txt"));
    ASSERT_FALSE(cmake.isEmpty());
    const QRegularExpressionMatch m =
        QRegularExpression(QStringLiteral("set\\(MEDIAFLYOUTS_TRANSLATED_LANGUAGES\\s+([^)]+)\\)")).match(cmake);
    ASSERT_TRUE(m.hasMatch()) << "MEDIAFLYOUTS_TRANSLATED_LANGUAGES not found in CMakeLists.txt";
    QStringList cmakeCodes = m.captured(1).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QStringList modelCodes = shippedCodes();
    cmakeCodes.sort();
    modelCodes.sort();
    EXPECT_EQ(cmakeCodes, modelCodes);
}

TEST(TranslationFilesTest, QmlStringsAndTsSourcesMatchPerFile) {
    // lupdate uses the QML file name as the context. A qsTr() string that is missing
    // from the .ts means update_translations was not run after a string change.
    const QMap<QString, QSet<QString>> ts = tsSourcesByContext();
    const QDir views(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/Views"));
    const QStringList files = views.entryList({QStringLiteral("*.qml")}, QDir::Files);
    ASSERT_FALSE(files.isEmpty());
    QSet<QString> qmlContexts;
    for(const QString& file : files) {
        const QString context = file.chopped(4);
        const QSet<QString> inQml = literalsOf(readSource(QStringLiteral("Views/") + file), QStringLiteral("qsTr\\("));
        if(!inQml.isEmpty())
            qmlContexts.insert(context);
        expectSameStrings(inQml, ts.value(context), context);
    }
    EXPECT_TRUE(qmlContexts.contains(QStringLiteral("Dashboard")));
}

TEST(TranslationFilesTest, CppStringsAndTsSourcesMatch) {
    // C++ contexts are class names, so compare the union across all non-QML contexts.
    const QDir views(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/Views"));
    QSet<QString> qmlContexts;
    for(const QString& file : views.entryList({QStringLiteral("*.qml")}, QDir::Files))
        qmlContexts.insert(file.chopped(4));

    QSet<QString> inTs;
    const QMap<QString, QSet<QString>> ts = tsSourcesByContext();
    for(auto it = ts.cbegin(); it != ts.cend(); ++it)
        if(!qmlContexts.contains(it.key()))
            inTs.unite(it.value());

    QSet<QString> inCpp;
    const QStringList roots = {QStringLiteral("Controllers"), QStringLiteral("Model"), QString()};
    for(const QString& root : roots) {
        const QString base = QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/") + root;
        QDirIterator it(base, {QStringLiteral("*.cpp")}, QDir::Files,
                        root.isEmpty() ? QDirIterator::NoIteratorFlags : QDirIterator::Subdirectories);
        while(it.hasNext()) {
            const QString path = it.next();
            const QString source = readSource(QDir(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR)).relativeFilePath(path));
            inCpp.unite(literalsOf(source, QStringLiteral("\\btr\\(")));
            inCpp.unite(literalsOf(source, QStringLiteral("translate\\(\\s*\"[^\"]+\"\\s*,")));
        }
    }
    ASSERT_FALSE(inCpp.isEmpty());
    expectSameStrings(inCpp, inTs, QStringLiteral("C++ sources"));
}

TEST(TranslationFilesTest, MostTranslationsDifferFromTheSource) {
    // Brand names, key names and "%1 px" legitimately stay the same. A file that
    // was filled by copying the English text fails here.
    for(const QString& code : shippedCodes()) {
        const TsFile ts = parseTs(tsPath(code));
        ASSERT_TRUE(ts.error.isEmpty()) << ts.error.toStdString();
        int identical = 0;
        for(const Message& m : ts.messages)
            identical += m.source == m.translation;
        EXPECT_LE(identical * 10, ts.messages.size()) << code.toStdString() << ": " << identical << " of "
                                                      << ts.messages.size() << " entries equal the source";
    }
}
