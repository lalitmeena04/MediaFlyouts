#include "ShowcaseDriver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHash>
#include <QImage>
#include <QQuickItem>
#include <QQuickWindow>

#include "AppTranslator.h"
#include "DemoArt.h"
#include "Localization.h"
#include "NowPlaying.h"

// Part grabs are debug lines. Enable with QT_LOGGING_RULES=mf.showcase.debug=true.
Q_LOGGING_CATEGORY(lcShowcase, "mf.showcase", QtInfoMsg)

class ShowcaseDriver::DemoPlayback : public mf::IPlaybackController {
    public:
        void play() override {}
        void pause() override {}
        void next() override {}
        void previous() override {}
        int sessionCount() const override {
            return sessions;
        }
        int selectedSessionIndex() const override {
            return selected;
        }

        int sessions = 1;
        int selected = 0;
};

ShowcaseDriver::ShowcaseDriver(NowPlaying* nowPlaying, AppTranslator* translator, ShowcaseOptions options,
                               QObject* parent)
    : QObject(parent), m_nowPlaying(nowPlaying), m_translator(translator), m_options(std::move(options)),
      m_playback(std::make_unique<DemoPlayback>()) {
    if(m_nowPlaying)
        m_nowPlaying->setController(m_playback.get());
}

ShowcaseDriver::~ShowcaseDriver() {
    if(m_nowPlaying)
        m_nowPlaying->setController(nullptr);
}

int ShowcaseDriver::partScale() const {
    return m_options.partScale;
}

int ShowcaseDriver::posterWidth() const {
    return m_options.posterSize.width();
}

int ShowcaseDriver::posterHeight() const {
    return m_options.posterSize.height();
}

QStringList ShowcaseDriver::languages() const {
    return m_options.languages;
}

QString ShowcaseDriver::language() const {
    return m_language;
}

QString ShowcaseDriver::captionFont() const {
    // The Windows UI font of each script. Left to Qt's fallback, one headline
    // can mix families of different weight and size.
    static const QHash<QString, QString> scriptFonts = {
        {QStringLiteral("ar"), QStringLiteral("Segoe UI")},
        {QStringLiteral("hi"), QStringLiteral("Nirmala UI")},
        {QStringLiteral("ja"), QStringLiteral("Yu Gothic UI")},
        {QStringLiteral("ko"), QStringLiteral("Malgun Gothic")},
        {QStringLiteral("zh_CN"), QStringLiteral("Microsoft YaHei UI")},
    };
    static const QStringList installed = QFontDatabase::families();
    const QString script = scriptFonts.value(m_language);
    if(!script.isEmpty() && installed.contains(script))
        return script;
    // Windows 11 font. Windows 10 has only the static family.
    static const QString preferred = QStringLiteral("Segoe UI Variable Display");
    return installed.contains(preferred) ? preferred : QStringLiteral("Segoe UI");
}

void ShowcaseDriver::setLanguage(const QString& code) {
    if(m_translator) {
        const bool source = code == mf::defaultLanguage();
        if(!source && !QFileInfo::exists(AppTranslator::appTranslationPath(code)))
            fail(QStringLiteral("no app translation for '%1'").arg(code));
        const QString effective = m_translator->apply(code);
        if(effective != code)
            fail(QStringLiteral("language '%1' resolved to '%2'").arg(code, effective));
    }
    qCInfo(lcShowcase).noquote() << "language" << code;
    if(m_language == code)
        return;
    m_language = code;
    emit languageChanged();
}

QString ShowcaseDriver::translate(const QString& context, const QString& text) const {
    return QCoreApplication::translate(context.toUtf8().constData(), text.toUtf8().constData());
}

QVariantMap ShowcaseDriver::caption(const QString& poster) {
    const QJsonObject entry = m_options.captions.value(m_language).toObject().value(poster).toObject();
    if(entry.isEmpty())
        fail(QStringLiteral("no caption for %1 in '%2'").arg(poster, m_language));
    return entry.toVariantMap();
}

int ShowcaseDriver::failures() const {
    return m_failures;
}

int ShowcaseDriver::saved() const {
    return m_saved;
}

bool ShowcaseDriver::wanted(const QString& name) const {
    return m_options.only.isEmpty() || m_options.only.contains(name, Qt::CaseInsensitive);
}

void ShowcaseDriver::setTrack(const QVariantMap& track) {
    if(!m_nowPlaying)
        return;
    m_playback->sessions = qMax(1, track.value(QStringLiteral("sessions"), 1).toInt());
    m_playback->selected = qBound(0, track.value(QStringLiteral("session"), 0).toInt(), m_playback->sessions - 1);

    mf::TrackInfo info;
    info.title = track.value(QStringLiteral("title")).toString();
    info.artist = track.value(QStringLiteral("artist")).toString();
    info.album = track.value(QStringLiteral("album")).toString();
    info.sourceApp = track.value(QStringLiteral("player")).toString() + QStringLiteral(".exe");
    info.state = track.value(QStringLiteral("playing"), true).toBool() ? mf::PlaybackState::Playing
                                                                        : mf::PlaybackState::Paused;
    info.positionMs = static_cast<qint64>(track.value(QStringLiteral("position")).toDouble() * 1000.0);
    info.durationMs = static_cast<qint64>(track.value(QStringLiteral("duration")).toDouble() * 1000.0);
    info.shuffle = track.value(QStringLiteral("shuffle")).toBool();
    info.repeatMode = track.value(QStringLiteral("repeat")).toInt();
    info.canSeek = true;
    info.canRepeat = true;
    info.canShuffle = true;

    m_nowPlaying->applyTrack(info);
    m_nowPlaying->applyArt(mf::demo::coverArt(track.value(QStringLiteral("cover")).toInt()));
    m_nowPlaying->noteSessionsChanged();
}

void ShowcaseDriver::clearTrack() {
    if(!m_nowPlaying)
        return;
    m_playback->sessions = 1;
    m_playback->selected = 0;
    m_nowPlaying->applyTrack(mf::TrackInfo{});
    m_nowPlaying->applyArt(QImage());
    m_nowPlaying->noteSessionsChanged();
}

QUrl ShowcaseDriver::appIconUrl(int index) {
    const QString folder = QDir(m_options.workDir).filePath(QStringLiteral("icons"));
    const QString path = QDir(folder).filePath(QStringLiteral("app-%1.png").arg(index));
    if(!QFileInfo::exists(path)) {
        if(!QDir().mkpath(folder) || !mf::demo::appIcon(index).save(path)) {
            fail(QStringLiteral("could not write %1").arg(path));
            return {};
        }
    }
    return QUrl::fromLocalFile(path);
}

QString ShowcaseDriver::partPath(const QString& name) const {
    return QDir(m_options.workDir).filePath(QStringLiteral("parts/%1/%2.png").arg(m_language, name));
}

bool ShowcaseDriver::prepare(QObject* window) {
    auto* quickWindow = qobject_cast<QQuickWindow*>(window);
    if(!quickWindow) {
        fail(QStringLiteral("prepare: not a window"));
        return false;
    }
    const auto fit = [quickWindow] {
        quickWindow->contentItem()->setSize(QSizeF(quickWindow->width(), quickWindow->height()));
    };
    connect(quickWindow, &QWindow::widthChanged, quickWindow, fit);
    connect(quickWindow, &QWindow::heightChanged, quickWindow, fit);
    fit();
    return true;
}

bool ShowcaseDriver::grabPart(QObject* window, const QString& name) {
    auto* quickWindow = qobject_cast<QQuickWindow*>(window);
    if(!quickWindow) {
        fail(QStringLiteral("part '%1' is not a window").arg(name));
        return false;
    }
    // The window is never shown. Qt Quick then renders one frame offscreen.
    const QImage image = quickWindow->grabWindow();
    if(image.isNull()) {
        fail(QStringLiteral("part '%1' rendered an empty image").arg(name));
        return false;
    }
    const QString path = partPath(name);
    if(!QDir().mkpath(QFileInfo(path).absolutePath()) || !image.save(path)) {
        fail(QStringLiteral("could not write %1").arg(path));
        return false;
    }
    ++m_saved;
    qCDebug(lcShowcase).noquote() << QStringLiteral("part %1: %2x%3 px (%4x%5 at %6x)")
                                        .arg(name)
                                        .arg(image.width())
                                        .arg(image.height())
                                        .arg(quickWindow->width())
                                        .arg(quickWindow->height())
                                        .arg(image.devicePixelRatio());
    return true;
}

QUrl ShowcaseDriver::partUrl(const QString& name) {
    const QString path = partPath(name);
    if(!QFileInfo::exists(path)) {
        fail(QStringLiteral("part '%1' is missing; run the parts stage first").arg(name));
        return {};
    }
    return QUrl::fromLocalFile(path);
}

bool ShowcaseDriver::savePoster(QObject* window, const QString& name) {
    auto* quickWindow = qobject_cast<QQuickWindow*>(window);
    if(!quickWindow) {
        fail(QStringLiteral("poster '%1' is not a window").arg(name));
        return false;
    }
    QImage image = quickWindow->grabWindow();
    if(image.isNull()) {
        fail(QStringLiteral("poster '%1' rendered an empty image").arg(name));
        return false;
    }
    if(image.size() != m_options.posterSize) {
        qCWarning(lcShowcase).noquote() << QStringLiteral("poster %1: got %2x%3, scaling to %4x%5")
                                               .arg(name)
                                               .arg(image.width())
                                               .arg(image.height())
                                               .arg(m_options.posterSize.width())
                                               .arg(m_options.posterSize.height());
        image = image.scaled(m_options.posterSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    // The Store shows screenshots opaque.
    image = image.convertToFormat(QImage::Format_RGB888);
    image.setDevicePixelRatio(1.0);

    const QString folder = QDir(m_options.outDir).filePath(m_language);
    const QString path = QDir(folder).filePath(name + QStringLiteral(".png"));
    if(!QDir().mkpath(folder) || !image.save(path)) {
        fail(QStringLiteral("could not write %1").arg(path));
        return false;
    }
    if(!m_options.previewDir.isEmpty()) {
        const QString preview =
            QDir(m_options.previewDir).filePath(QStringLiteral("%1-%2.jpg").arg(m_language, name));
        const QImage small = image.scaled(image.size() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if(!QDir().mkpath(m_options.previewDir) || !small.save(preview, "JPG", 88))
            qCWarning(lcShowcase).noquote() << "could not write preview" << preview;
    }
    ++m_saved;
    qCInfo(lcShowcase).noquote() << QStringLiteral("poster %1/%2: %3 (%4 KB)")
                                        .arg(m_language, name, QDir::toNativeSeparators(path))
                                        .arg(QFileInfo(path).size() / 1024);
    return true;
}

void ShowcaseDriver::fail(const QString& reason) {
    ++m_failures;
    qCCritical(lcShowcase).noquote() << "FAILED:" << reason;
}

void ShowcaseDriver::finish() {
    QCoreApplication::exit(m_failures == 0 ? 0 : 1);
}
