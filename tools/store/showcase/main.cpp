// Renders Store screenshots from the real QML views with demo data.
//
// Stage "parts": each flyout and widget is grabbed offscreen at a high device
// pixel ratio, once per language with the app translation loaded. Stage
// "posters": the grabs are composed into 16:9 images at native resolution,
// with the caption of that language. The two stages need different scale
// factors, and Qt fixes the factor at startup, so the default run starts the
// poster stage as a child process.

#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonObject>
#include <QMetaProperty>
#include <QProcess>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>

#include <cstring>
#include <memory>

#include "AlbumArtProvider.h"
#include "AppTranslator.h"
#include "Captions.h"
#include "Localization.h"
#include "NowPlaying.h"
#include "Settings.h"
#include "ShowcaseDriver.h"
#include "ThemeController.h"

namespace {

constexpr int kPartScale = 6;
constexpr int kStageTimeoutPerLanguageMs = 120000;
const QString kPartsStage = QStringLiteral("parts");
const QString kPostersStage = QStringLiteral("posters");

// Read before QGuiApplication exists.
QString rawStage(int argc, char** argv) {
    for(int i = 1; i + 1 < argc; ++i)
        if(std::strcmp(argv[i], "--stage") == 0)
            return QString::fromLocal8Bit(argv[i + 1]);
    return {};
}

bool samePathPrefix(const QString& path, const QString& prefix) {
    return QDir::cleanPath(path).startsWith(QDir::cleanPath(prefix), Qt::CaseInsensitive);
}

// Copies Views/*.qml into <importRoot>/MediaFlyouts and writes a qmldir, so
// "import MediaFlyouts" resolves without the application binary.
bool stageViewsModule(const QString& viewsDir, const QString& importRoot) {
    const QString moduleDir = QDir(importRoot).filePath(QStringLiteral("MediaFlyouts"));
    if(!QDir().mkpath(moduleDir))
        return false;

    QString qmldir = QStringLiteral("module MediaFlyouts\n");
    const QFileInfoList files = QDir(viewsDir).entryInfoList({QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    for(const QFileInfo& file : files) {
        QFile source(file.absoluteFilePath());
        if(!source.open(QIODevice::ReadOnly))
            return false;
        const QByteArray content = source.readAll();
        QFile target(QDir(moduleDir).filePath(file.fileName()));
        if(!target.open(QIODevice::WriteOnly) || target.write(content) != content.size())
            return false;
        const bool singleton = content.left(512).contains("pragma Singleton");
        qmldir += QStringLiteral("%1%2 1.0 %3\n")
                      .arg(singleton ? QStringLiteral("singleton ") : QString(), file.completeBaseName(), file.fileName());
    }
    QFile out(QDir(moduleDir).filePath(QStringLiteral("qmldir")));
    if(!out.open(QIODevice::WriteOnly))
        return false;
    out.write(qmldir.toUtf8());
    qCInfo(lcShowcase) << "staged" << files.size() << "views";
    return !files.isEmpty();
}

QObject* createFrom(QQmlEngine& engine, const QString& path) {
    QQmlComponent component(&engine, QUrl::fromLocalFile(path));
    QObject* object = component.create();
    if(!object) {
        for(const QQmlError& error : component.errors())
            qCCritical(lcShowcase).noquote() << "QML:" << error.toString();
    }
    return object;
}

// Every object-valued property of the root becomes a context property of the same name.
int exposeChildren(QQmlContext* context, QObject* root) {
    int count = 0;
    const QMetaObject* meta = root->metaObject();
    for(int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        const QMetaProperty property = meta->property(i);
        if(QObject* object = property.read(root).value<QObject*>()) {
            context->setContextProperty(QString::fromLatin1(property.name()), object);
            ++count;
        }
    }
    return count;
}

int stageTimeoutMs(const ShowcaseOptions& options) {
    return kStageTimeoutPerLanguageMs * static_cast<int>(qMax<qsizetype>(1, options.languages.size()));
}

int runStage(QGuiApplication& app, const ShowcaseOptions& options) {
    const QString sourceDir = QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR);
    const QString qmlDir = QDir(sourceDir).filePath(QStringLiteral("tools/store/showcase/qml"));
    const bool parts = options.stage == kPartsStage;

    // Settings must never touch the user's own file.
    QTemporaryDir scratch;
    if(!scratch.isValid()) {
        qCCritical(lcShowcase) << "no scratch folder";
        return 1;
    }
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, scratch.filePath(QStringLiteral("settings")));

    // Declared before the engine: context objects must outlive the QML objects.
    std::unique_ptr<Settings> settings;
    std::unique_ptr<NowPlaying> nowPlaying;
    std::unique_ptr<ThemeController> theme;
    std::unique_ptr<AppTranslator> translator;
    if(parts) {
        settings = std::make_unique<Settings>();
        if(!samePathPrefix(settings->configDir(), scratch.path())) {
            qCCritical(lcShowcase).noquote() << "settings are not isolated:" << settings->configDir();
            return 1;
        }
        nowPlaying = std::make_unique<NowPlaying>();
        theme = std::make_unique<ThemeController>(settings.get(), nowPlaying.get());
        // Also installs the script fallback fonts, as the app does.
        translator = std::make_unique<AppTranslator>();
    }
    ShowcaseDriver driver(nowPlaying.get(), translator.get(), options);

    QQmlEngine engine;
    if(translator)
        translator->setEngine(&engine);
    int qmlWarnings = 0;
    engine.setOutputWarningsToStandardError(false);
    QObject::connect(&engine, &QQmlEngine::warnings, &app, [&qmlWarnings](const QList<QQmlError>& warnings) {
        for(const QQmlError& warning : warnings) {
            ++qmlWarnings;
            qCWarning(lcShowcase).noquote() << "QML:" << warning.toString();
        }
    });

    QQmlContext* context = engine.rootContext();
    context->setContextProperty(QStringLiteral("showcase"), &driver);

    QString directorFile = QStringLiteral("PostersDirector.qml");
    if(parts) {
        directorFile = QStringLiteral("PartsDirector.qml");
        auto* artProvider = new AlbumArtProvider(); // The engine owns it.
        engine.addImageProvider(QStringLiteral("albumart"), artProvider);
        nowPlaying->setArtProvider(artProvider);
        context->setContextProperty(QStringLiteral("settings"), settings.get());
        context->setContextProperty(QStringLiteral("nowPlaying"), nowPlaying.get());
        context->setContextProperty(QStringLiteral("theme"), theme.get());

        const QString importRoot = scratch.filePath(QStringLiteral("import"));
        if(!stageViewsModule(QDir(sourceDir).filePath(QStringLiteral("Views")), importRoot)) {
            qCCritical(lcShowcase) << "could not stage the views module";
            return 1;
        }
        engine.addImportPath(importRoot);

        QObject* world = createFrom(engine, QDir(qmlDir).filePath(QStringLiteral("Fakes.qml")));
        if(!world)
            return 1;
        world->setParent(&engine);
        context->setContextProperty(QStringLiteral("world"), world);
        qCInfo(lcShowcase) << "exposed" << exposeChildren(context, world) << "stand-in controllers";
    }

    QTimer::singleShot(stageTimeoutMs(options), &driver, [&driver] {
        driver.fail(QStringLiteral("stage timed out"));
        driver.finish();
    });

    QObject* director = createFrom(engine, QDir(qmlDir).filePath(directorFile));
    if(!director)
        return 1;
    director->setParent(&engine);

    const int code = app.exec();
    qCInfo(lcShowcase).noquote() << QStringLiteral("stage %1: %2 image(s), %3 failure(s), %4 QML warning(s)")
                                        .arg(options.stage)
                                        .arg(driver.saved())
                                        .arg(driver.failures())
                                        .arg(qmlWarnings);
    // A QML warning usually means a view reads something the stand-ins lack.
    return (code == 0 && qmlWarnings == 0 && driver.saved() > 0) ? 0 : 1;
}

} // namespace

int main(int argc, char** argv) {
    const QString requested = rawStage(argc, argv);
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    qputenv("QT_SCALE_FACTOR", requested == kPostersStage ? QByteArray("1") : QByteArray::number(kPartScale));
    // Without this, Qt sends log lines to the debugger when stderr is a pipe.
    qputenv("QT_FORCE_STDERR_LOGGING", "1");
    qSetMessagePattern(QStringLiteral("[%{type}] %{message}"));

    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("MediaFlyouts"));
    QCoreApplication::setOrganizationName(QStringLiteral("MediaFlyouts"));

    const QString sourceDir = QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR);
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Renders MediaFlyouts Store screenshots."));
    parser.addHelpOption();
    QCommandLineOption outOption(QStringLiteral("out"), QStringLiteral("Poster output folder."), QStringLiteral("dir"),
                                 QDir(sourceDir).filePath(QStringLiteral("tools/store/screenshots/showcase")));
    QCommandLineOption workOption(QStringLiteral("work"), QStringLiteral("Folder for grabbed views."),
                                  QStringLiteral("dir"), QDir(sourceDir).filePath(QStringLiteral("build/showcase")));
    QCommandLineOption previewOption(QStringLiteral("preview"), QStringLiteral("Also write half-size JPEG copies here."),
                                     QStringLiteral("dir"));
    QCommandLineOption onlyOption(QStringLiteral("only"), QStringLiteral("Comma-separated poster names."),
                                  QStringLiteral("names"));
    QCommandLineOption langsOption(QStringLiteral("langs"),
                                   QStringLiteral("Comma-separated app language codes. Default: every shipped language."),
                                   QStringLiteral("codes"));
    QCommandLineOption stageOption(QStringLiteral("stage"), QStringLiteral("Run one stage: parts or posters."),
                                   QStringLiteral("stage"));
    QCommandLineOption checkOption(QStringLiteral("check"),
                                   QStringLiteral("Render everything into a scratch folder and report failures."));
    parser.addOptions({outOption, workOption, previewOption, onlyOption, langsOption, stageOption, checkOption});
    parser.process(app);

    QTemporaryDir checkDir;
    ShowcaseOptions options;
    options.partScale = kPartScale;
    options.outDir = parser.value(outOption);
    options.workDir = parser.value(workOption);
    options.previewDir = parser.value(previewOption);
    options.only = parser.value(onlyOption).split(QLatin1Char(','), Qt::SkipEmptyParts);
    if(parser.isSet(checkOption)) {
        options.outDir = checkDir.filePath(QStringLiteral("out"));
        options.workDir = checkDir.filePath(QStringLiteral("work"));
        options.previewDir.clear();
    }

    // Captions must cover every shipped language, also when fewer are rendered.
    QStringList shipped;
    for(const mf::LanguageInfo& info : mf::supportedLanguages())
        shipped << info.code;
    QString captionError;
    options.captions = mf::showcase::loadCaptions(
        QDir(sourceDir).filePath(QStringLiteral("tools/store/showcase/captions.json")), &captionError);
    QStringList captionProblems = mf::showcase::captionProblems(options.captions, shipped);
    if(!captionError.isEmpty())
        captionProblems = {captionError};
    if(!captionProblems.isEmpty()) {
        for(const QString& problem : std::as_const(captionProblems))
            qCCritical(lcShowcase).noquote() << "captions:" << problem;
        return 1;
    }
    options.languages = parser.value(langsOption).split(QLatin1Char(','), Qt::SkipEmptyParts);
    if(options.languages.isEmpty())
        options.languages = shipped;
    for(const QString& language : std::as_const(options.languages)) {
        if(!shipped.contains(language)) {
            qCCritical(lcShowcase).noquote() << "unknown language:" << language << "- shipped:" << shipped.join(' ');
            return 2;
        }
    }

    const QString stage = parser.value(stageOption);
    if(!stage.isEmpty() && stage != kPartsStage && stage != kPostersStage) {
        qCCritical(lcShowcase).noquote() << "unknown stage:" << stage;
        return 2;
    }
    if(!stage.isEmpty()) {
        options.stage = stage;
        return runStage(app, options);
    }

    options.stage = kPartsStage;
    if(const int code = runStage(app, options); code != 0)
        return code;

    QStringList arguments{QStringLiteral("--stage"), kPostersStage, QStringLiteral("--out"), options.outDir,
                          QStringLiteral("--work"), options.workDir, QStringLiteral("--langs"),
                          options.languages.join(QLatin1Char(','))};
    if(!options.previewDir.isEmpty())
        arguments << QStringLiteral("--preview") << options.previewDir;
    if(!options.only.isEmpty())
        arguments << QStringLiteral("--only") << options.only.join(QLatin1Char(','));
    QProcess child;
    child.setProcessChannelMode(QProcess::ForwardedChannels);
    child.start(QCoreApplication::applicationFilePath(), arguments);
    if(!child.waitForFinished(stageTimeoutMs(options) + 30000) || child.exitStatus() != QProcess::NormalExit) {
        qCCritical(lcShowcase) << "poster stage did not finish";
        return 1;
    }
    return child.exitCode();
}
