#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class QDateTime;
class Settings;
class Capabilities;

// Bundles logs, settings and system facts into one report the user can attach
// to a GitHub issue. Nothing leaves the PC unless the user posts it.
class ErrorReporter : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)
        Q_PROPERTY(QString lastReportPath READ lastReportPath NOTIFY changed)
        Q_PROPERTY(QString issueUrl READ issueUrl CONSTANT)

    public:
        explicit ErrorReporter(const Settings* settings, const Capabilities* capabilities, QObject* parent = nullptr);

        QString status() const;
        // Zip file, or folder when no archiver was available. Empty before the first report.
        QString lastReportPath() const;

        // New-issue page on GitHub.
        static QString issueUrl();
        // New-issue page with a prefilled body that names the report file.
        static QString issueUrlFor(const QString& reportPath, const QString& summary);
        // Root folder that holds generated reports.
        static QString reportsDir();
        // Reports kept on disk. Older ones are removed after each new report.
        static constexpr int kReportsKept = 3;

        // Plain-text facts about the app, OS, displays and capabilities.
        static QString systemSummary(const Settings* settings, const Capabilities* capabilities);
        // Write report.txt, settings.ini and logs/*.log into targetDir.
        // Returns the relative paths written. Empty on failure.
        static QStringList writeReport(const QString& targetDir, const QString& logDir, const Settings* settings,
                                       const Capabilities* capabilities);
        // Remove all but the newest `keep` report entries under dir. Returns removed count.
        static int pruneReports(const QString& dir, int keep);
        // Zip the folder with the OS archiver and delete the folder. Empty when not possible.
        static QString compressFolder(const QString& folder);
        // Folder or file name for a report made at the given time.
        static QString reportName(const QDateTime& when);

    public slots:
        // Build a report now. Sets lastReportPath and status.
        void createReport();
        // Show the report in the file manager.
        void revealReport();
        // Open the GitHub new-issue page in the browser.
        void openIssuePage();

    signals:
        void changed();

    private:
        const Settings* m_settings = nullptr;
        const Capabilities* m_capabilities = nullptr;
        QString m_status = QStringLiteral("No report created yet.");
        QString m_lastReportPath;
};
