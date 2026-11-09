#include "ErrorReporter.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QProcess>
#include <QScreen>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>

#include "Capabilities.h"
#include "InstallSource.h"
#include "Logging.h"
#include "Settings.h"
#include "Version.h"

namespace {
constexpr char kIssueUrl[] = "https://github.com/lalitmeena04/MediaFlyouts-Releases/issues/new";
constexpr char kReportPrefix[] = "MediaFlyouts-report-";
} // namespace

ErrorReporter::ErrorReporter(const Settings* settings, const Capabilities* capabilities, QObject* parent)
    : QObject(parent), m_settings(settings), m_capabilities(capabilities) {}

QString ErrorReporter::status() const {
    return m_status;
}

QString ErrorReporter::lastReportPath() const {
    return m_lastReportPath;
}

QString ErrorReporter::issueUrl() {
    return QString::fromLatin1(kIssueUrl);
}

QString ErrorReporter::issueUrlFor(const QString& reportPath) {
    const QString body = QStringLiteral("**What happened?**\n\n\n"
                                        "**Steps to reproduce**\n\n1. \n2. \n\n"
                                        "_Drag the report file into this issue: `%1`. "
                                        "It has the app version, OS and system details._\n")
                             .arg(QFileInfo(reportPath).fileName());
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("title"), QStringLiteral("[Bug] "));
    query.addQueryItem(QStringLiteral("body"), body);
    QUrl url(issueUrl());
    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

QString ErrorReporter::reportsDir() {
    return firstWritableDir(reportRootCandidates());
}

QStringList ErrorReporter::reportRootCandidates() {
    QStringList roots;
    const QString sub = QStringLiteral("/MediaFlyouts/reports");
    const QString programData = QDir::fromNativeSeparators(qEnvironmentVariable("ProgramData"));
    if(!programData.isEmpty())
        roots << QDir::cleanPath(programData + sub);
    const QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if(!downloads.isEmpty())
        roots << QDir::cleanPath(downloads + sub);
    roots << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reports");
    return roots;
}

QString ErrorReporter::firstWritableDir(const QStringList& candidates) {
    for(const QString& dir : candidates) {
        if(dir.isEmpty())
            continue;
        if(!QDir().mkpath(dir)) {
            TRC_DEBUG("report: cannot create {}", dir.toStdString());
            continue;
        }
        // A probe file proves write access. ACLs can deny it on managed PCs.
        QTemporaryFile probe(dir + QStringLiteral("/.probe-XXXXXX"));
        if(probe.open())
            return dir;
        TRC_DEBUG("report: not writable {}", dir.toStdString());
    }
    return {};
}

QString ErrorReporter::reportName(const QDateTime& when) {
    return QString::fromLatin1(kReportPrefix) + when.toString(QStringLiteral("yyyyMMdd-HHmmss"));
}

QString ErrorReporter::systemSummary(const Settings* settings, const Capabilities* capabilities) {
    QString text;
    QTextStream out(&text);
    out << "App version: " << MEDIAFLYOUTS_VERSION << '\n';
    out << "Qt: built " << QT_VERSION_STR << ", running " << qVersion() << '\n';
    out << "OS: " << QSysInfo::prettyProductName() << " (kernel " << QSysInfo::kernelVersion() << ", "
        << QSysInfo::currentCpuArchitecture() << ")\n";
    out << "Locale: " << QLocale::system().name() << '\n';

    // Screens need a GUI application. Tests run without one.
    if(auto* gui = qobject_cast<QGuiApplication*>(QCoreApplication::instance())) {
        const QList<QScreen*> screens = gui->screens();
        out << "Screens: " << screens.size() << '\n';
        for(int i = 0; i < screens.size(); ++i) {
            const QScreen* s = screens.at(i);
            const QRect g = s->geometry();
            out << "  [" << i << "] " << s->name() << " " << g.width() << 'x' << g.height() << " at " << g.x() << ','
                << g.y() << " scale " << s->devicePixelRatio() << (s == gui->primaryScreen() ? " (primary)" : "")
                << '\n';
        }
    } else {
        out << "Screens: n/a\n";
    }

    if(capabilities) {
        out << "Capabilities: " << capabilities->statusSummary() << '\n';
        if(capabilities->degraded())
            out << "Unavailable: " << capabilities->unavailableNames().join(QStringLiteral(", ")) << '\n';
    }
    if(settings)
        out << "Settings file: " << mf::hostVisiblePath(settings->configDir()) << '\n';
    return text;
}

QStringList ErrorReporter::writeReport(const QString& targetDir, const QString& logDir, const Settings* settings,
                                       const Capabilities* capabilities) {
    QStringList written;
    QDir dir(targetDir);
    if(!dir.mkpath(QStringLiteral("."))) {
        TRC_WARN("report: cannot create {}", targetDir.toStdString());
        return written;
    }

    QFile report(dir.filePath(QStringLiteral("report.txt")));
    if(!report.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRC_WARN("report: cannot write report.txt");
        return written;
    }
    {
        QTextStream out(&report);
        out << "MediaFlyouts error report\n";
        out << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
        out << systemSummary(settings, capabilities);
    }
    report.close();
    written << QStringLiteral("report.txt");

    if(settings && settings->exportTo(QUrl::fromLocalFile(dir.filePath(QStringLiteral("settings.ini")))))
        written << QStringLiteral("settings.ini");

    if(!logDir.isEmpty()) {
        mf::flushLogging();
        const QDir logs(logDir);
        const QStringList files = logs.entryList({QStringLiteral("*.log")}, QDir::Files, QDir::Name);
        if(!files.isEmpty())
            dir.mkpath(QStringLiteral("logs"));
        for(const QString& file : files) {
            const QString rel = QStringLiteral("logs/") + file;
            if(QFile::copy(logs.filePath(file), dir.filePath(rel)))
                written << rel;
            else
                TRC_WARN("report: cannot copy {}", file.toStdString());
        }
    }
    return written;
}

int ErrorReporter::pruneReports(const QString& dir, int keep) {
    const QDir root(dir);
    QStringList entries = root.entryList({QString::fromLatin1(kReportPrefix) + QLatin1Char('*')},
                                         QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    int removed = 0;
    while(entries.size() > qMax(keep, 0)) {
        const QString oldest = root.filePath(entries.takeFirst());
        const bool ok = QFileInfo(oldest).isDir() ? QDir(oldest).removeRecursively() : QFile::remove(oldest);
        if(ok)
            ++removed;
    }
    return removed;
}

QString ErrorReporter::compressFolder(const QString& folder) {
    const QString tar = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if(tar.isEmpty())
        return {};
    const QString zip = folder + QStringLiteral(".zip");
    QFile::remove(zip);
    QProcess proc;
    // -a picks zip from the extension. bsdtar on Windows 10+ supports it.
    proc.start(tar, {QStringLiteral("-a"), QStringLiteral("-c"), QStringLiteral("-f"), QDir::toNativeSeparators(zip),
                     QStringLiteral("-C"), QDir::toNativeSeparators(folder), QStringLiteral(".")});
    if(!proc.waitForFinished(15000) || proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0
       || !QFileInfo::exists(zip)) {
        TRC_WARN("report: zip failed (exit={}): {}", proc.exitCode(), QString::fromLocal8Bit(proc.readAllStandardError()).trimmed().toStdString());
        QFile::remove(zip);
        return {};
    }
    QDir(folder).removeRecursively();
    return zip;
}

void ErrorReporter::createReport() {
    const QString root = reportsDir();
    if(root.isEmpty()) {
        TRC_WARN("report: no writable folder among {}", reportRootCandidates().join(QStringLiteral(", ")).toStdString());
        m_status = tr("Couldn't find a writable folder for the report.");
        emit changed();
        return;
    }
    const QString folder = root + QLatin1Char('/') + reportName(QDateTime::currentDateTime());
    TRC_INFO("report: creating {}", folder.toStdString());

    const QStringList files = writeReport(folder, mf::logDir(), m_settings, m_capabilities);
    if(files.isEmpty()) {
        m_status = tr("Couldn't write the report. Check that %1 is writable.").arg(QDir::toNativeSeparators(root));
        emit changed();
        return;
    }

    QString result = compressFolder(folder);
    if(result.isEmpty())
        result = folder;
    pruneReports(root, kReportsKept);

    m_lastReportPath = QDir::toNativeSeparators(result);
    m_status = tr("Report saved with %1 files.").arg(files.size());
    TRC_INFO("report: saved {}", result.toStdString());
    emit changed();
}

void ErrorReporter::revealReport() {
    if(m_lastReportPath.isEmpty())
        return;
    QString target = m_lastReportPath;
    if(!QFileInfo::exists(target)) {
        // Pruned or deleted by hand. Show the folder instead of letting Explorer guess.
        TRC_WARN("report: {} is gone; opening the reports folder", target.toStdString());
        target = reportsDir();
        if(target.isEmpty())
            return;
    }
    TRC_INFO("report: reveal {}", target.toStdString());
#ifdef Q_OS_WIN
    if(QFileInfo(target).isFile()
       && QProcess::startDetached(QStringLiteral("explorer.exe"),
                                  {QStringLiteral("/select,"), QDir::toNativeSeparators(target)}))
        return;
#endif
    const QFileInfo info(target);
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir() ? info.absoluteFilePath() : info.absolutePath()));
}

void ErrorReporter::openIssuePage() {
    const QString url = m_lastReportPath.isEmpty() ? issueUrl() : issueUrlFor(m_lastReportPath);
    QDesktopServices::openUrl(QUrl(url));
}
