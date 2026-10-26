#include <gtest/gtest.h>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <QUrlQuery>

#include "Capabilities.h"
#include "ErrorReporter.h"
#include "Settings.h"
#include "Version.h"

namespace {
class ErrorReporterTest : public ::testing::Test {
    protected:
        void SetUp() override {
            QStandardPaths::setTestModeEnabled(true);
            QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                      QStringLiteral("MediaFlyouts"))
                .clear();
        }
};

void writeFile(const QString& path, const QByteArray& bytes) {
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(bytes);
}
} // namespace

TEST_F(ErrorReporterTest, SummaryNamesVersionOsAndCapabilities) {
    Settings settings;
    Capabilities caps;
    caps.setAudioAvailable(false);

    const QString summary = ErrorReporter::systemSummary(&settings, &caps);
    EXPECT_TRUE(summary.contains(QStringLiteral(MEDIAFLYOUTS_VERSION)));
    EXPECT_TRUE(summary.contains(QStringLiteral("OS: ")));
    EXPECT_TRUE(summary.contains(QStringLiteral("Qt: ")));
    EXPECT_TRUE(summary.contains(QStringLiteral("Unavailable: ")));
    EXPECT_TRUE(summary.contains(QStringLiteral("Screens: n/a")));
}

TEST_F(ErrorReporterTest, SummaryToleratesMissingSources) {
    const QString summary = ErrorReporter::systemSummary(nullptr, nullptr);
    EXPECT_FALSE(summary.isEmpty());
    EXPECT_FALSE(summary.contains(QStringLiteral("Capabilities:")));
    EXPECT_FALSE(summary.contains(QStringLiteral("Settings file:")));
}

TEST_F(ErrorReporterTest, ReportBundlesSummarySettingsAndLogs) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString logDir = temp.filePath(QStringLiteral("logs"));
    ASSERT_TRUE(QDir().mkpath(logDir));
    writeFile(logDir + QStringLiteral("/mediaflyouts.log"), "line one\n");
    writeFile(logDir + QStringLiteral("/mediaflyouts.1.log"), "older\n");
    writeFile(logDir + QStringLiteral("/notes.txt"), "not a log\n");

    Settings settings;
    settings.setAutoCheckUpdates(true);
    Capabilities caps;
    const QString target = temp.filePath(QStringLiteral("report"));

    const QStringList written = ErrorReporter::writeReport(target, logDir, &settings, &caps);
    EXPECT_TRUE(written.contains(QStringLiteral("report.txt")));
    EXPECT_TRUE(written.contains(QStringLiteral("settings.ini")));
    EXPECT_TRUE(written.contains(QStringLiteral("logs/mediaflyouts.log")));
    EXPECT_TRUE(written.contains(QStringLiteral("logs/mediaflyouts.1.log")));
    EXPECT_FALSE(written.contains(QStringLiteral("logs/notes.txt")));
    EXPECT_TRUE(QFile::exists(target + QStringLiteral("/logs/mediaflyouts.log")));

    QSettings exported(target + QStringLiteral("/settings.ini"), QSettings::IniFormat);
    EXPECT_TRUE(exported.value(QStringLiteral("autoCheckUpdates")).toBool());

    QFile report(target + QStringLiteral("/report.txt"));
    ASSERT_TRUE(report.open(QIODevice::ReadOnly));
    const QString text = QString::fromUtf8(report.readAll());
    EXPECT_TRUE(text.contains(QStringLiteral("MediaFlyouts error report")));
    EXPECT_TRUE(text.contains(QStringLiteral(MEDIAFLYOUTS_VERSION)));
}

TEST_F(ErrorReporterTest, ReportWorksWithoutLogs) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QStringList written =
        ErrorReporter::writeReport(temp.filePath(QStringLiteral("r")), QString(), nullptr, nullptr);
    EXPECT_EQ(written, QStringList{QStringLiteral("report.txt")});
}

TEST_F(ErrorReporterTest, PruneKeepsOnlyTheNewestReports) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QDir root(temp.path());
    for(int day = 1; day <= 5; ++day) {
        const QString name = ErrorReporter::reportName(QDateTime(QDate(2026, 1, day), QTime(12, 0)));
        if(day % 2)
            ASSERT_TRUE(root.mkpath(name));
        else
            writeFile(root.filePath(name + QStringLiteral(".zip")), "zip");
    }
    writeFile(root.filePath(QStringLiteral("unrelated.txt")), "keep me");

    EXPECT_EQ(ErrorReporter::pruneReports(temp.path(), ErrorReporter::kReportsKept), 2);
    const QStringList left = root.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    EXPECT_EQ(left.size(), 4);
    EXPECT_TRUE(left.contains(QStringLiteral("unrelated.txt")));
    EXPECT_FALSE(left.contains(QStringLiteral("MediaFlyouts-report-20260101-120000")));
    EXPECT_FALSE(left.contains(QStringLiteral("MediaFlyouts-report-20260102-120000.zip")));
    EXPECT_TRUE(left.contains(QStringLiteral("MediaFlyouts-report-20260105-120000")));

    EXPECT_EQ(ErrorReporter::pruneReports(temp.path(), 0), 3);
}

TEST_F(ErrorReporterTest, ReportNameIsSortableByTime) {
    const QString a = ErrorReporter::reportName(QDateTime(QDate(2026, 3, 9), QTime(8, 5, 7)));
    const QString b = ErrorReporter::reportName(QDateTime(QDate(2026, 3, 9), QTime(8, 5, 8)));
    EXPECT_EQ(a, QStringLiteral("MediaFlyouts-report-20260309-080507"));
    EXPECT_LT(a, b);
}

TEST_F(ErrorReporterTest, IssueUrlCarriesOnlyTheReportFileName) {
    const QString url = ErrorReporter::issueUrlFor(QStringLiteral("C:/Users/someone/x/MediaFlyouts-report-1.zip"));
    EXPECT_TRUE(url.startsWith(ErrorReporter::issueUrl()));

    const QUrlQuery query(QUrl(url).query(QUrl::FullyEncoded));
    const QString body = query.queryItemValue(QStringLiteral("body"), QUrl::FullyDecoded);
    EXPECT_TRUE(body.contains(QStringLiteral("MediaFlyouts-report-1.zip")));
    EXPECT_FALSE(body.contains(QStringLiteral("someone")));
    EXPECT_FALSE(body.contains(QStringLiteral("App version:")));
    EXPECT_FALSE(body.contains(QStringLiteral("OS:")));
    EXPECT_FALSE(body.contains(QStringLiteral("Settings file:")));
    EXPECT_FALSE(url.contains(QStringLiteral(MEDIAFLYOUTS_VERSION)));
}

TEST_F(ErrorReporterTest, CompressFolderProducesAZipWhenAnArchiverExists) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString folder = temp.filePath(QStringLiteral("MediaFlyouts-report-x"));
    ASSERT_TRUE(QDir().mkpath(folder));
    writeFile(folder + QStringLiteral("/report.txt"), "hello");

    const QString zip = ErrorReporter::compressFolder(folder);
    if(zip.isEmpty()) {
        EXPECT_TRUE(QFile::exists(folder + QStringLiteral("/report.txt")));
        return;
    }
    EXPECT_EQ(zip, folder + QStringLiteral(".zip"));
    EXPECT_TRUE(QFile::exists(zip));
    EXPECT_FALSE(QDir(folder).exists());
}

TEST_F(ErrorReporterTest, DefaultStateBeforeAnyReport) {
    ErrorReporter reporter(nullptr, nullptr);
    EXPECT_TRUE(reporter.lastReportPath().isEmpty());
    EXPECT_FALSE(reporter.status().isEmpty());
    EXPECT_EQ(ErrorReporter::issueUrl(), QStringLiteral("https://github.com/lalitmeena04/MediaFlyouts/issues/new"));
}

TEST_F(ErrorReporterTest, FirstWritableDirSkipsFoldersItCannotCreate) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    // A path below a regular file can never be created.
    writeFile(temp.filePath(QStringLiteral("blocker")), "x");
    const QString blocked = temp.filePath(QStringLiteral("blocker/reports"));
    const QString good = temp.filePath(QStringLiteral("ok/MediaFlyouts/reports"));

    EXPECT_EQ(ErrorReporter::firstWritableDir({QString(), blocked, good}), good);
    EXPECT_TRUE(QDir(good).exists());
    EXPECT_TRUE(QDir(good).entryList(QDir::Files | QDir::Hidden).isEmpty()) << "probe file must be removed";
}

TEST_F(ErrorReporterTest, FirstWritableDirIsEmptyWhenNothingWorks) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    writeFile(temp.filePath(QStringLiteral("blocker")), "x");
    EXPECT_TRUE(ErrorReporter::firstWritableDir({temp.filePath(QStringLiteral("blocker/a")), QString()}).isEmpty());
    EXPECT_TRUE(ErrorReporter::firstWritableDir({}).isEmpty());
}

TEST_F(ErrorReporterTest, ReportRootsPreferSharedDiskOverAppData) {
    const QStringList roots = ErrorReporter::reportRootCandidates();
    ASSERT_GE(roots.size(), 2);
    // Store builds get AppData redirected into the package. Keep it last.
    EXPECT_TRUE(roots.last().endsWith(QStringLiteral("/reports")));
    EXPECT_EQ(roots.last(),
              QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/reports"));
    for(int i = 0; i < roots.size() - 1; ++i) {
        EXPECT_TRUE(roots.at(i).endsWith(QStringLiteral("/MediaFlyouts/reports"))) << roots.at(i).toStdString();
        EXPECT_FALSE(roots.at(i).contains(QStringLiteral("AppData"), Qt::CaseInsensitive)) << roots.at(i).toStdString();
    }
#ifdef Q_OS_WIN
    EXPECT_TRUE(roots.first().startsWith(QDir::fromNativeSeparators(qEnvironmentVariable("ProgramData")),
                                         Qt::CaseInsensitive))
        << roots.first().toStdString();
#endif
}
