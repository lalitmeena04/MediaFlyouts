#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class QDateTime;
class Settings;
class Capabilities;

class ErrorReporter : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)
        Q_PROPERTY(QString lastReportPath READ lastReportPath NOTIFY changed)
        Q_PROPERTY(QString issueUrl READ issueUrl CONSTANT)

    public:
        explicit ErrorReporter(const Settings* settings, const Capabilities* capabilities, QObject* parent = nullptr);

        QString status() const;
        QString lastReportPath() const;

        static QString issueUrl();
        static QString issueUrlFor(const QString& reportPath);
        // First writable folder from reportRootCandidates().
        static QString reportsDir();
        // Folders to try, best first. ProgramData and Downloads are real disk
        // locations in every install kind. Packaged (Store) apps get AppData
        // redirected into the package, where Explorer cannot find it.
        static QStringList reportRootCandidates();
        static QString firstWritableDir(const QStringList& candidates);
        static constexpr int kReportsKept = 3;

        static QString systemSummary(const Settings* settings, const Capabilities* capabilities);
        static QStringList writeReport(const QString& targetDir, const QString& logDir, const Settings* settings,
                                       const Capabilities* capabilities);
        static int pruneReports(const QString& dir, int keep);
        static QString compressFolder(const QString& folder);
        static QString reportName(const QDateTime& when);

    public slots:
        void createReport();
        void revealReport();
        void openIssuePage();

    signals:
        void changed();

    private:
        const Settings* m_settings = nullptr;
        const Capabilities* m_capabilities = nullptr;
        QString m_status = QStringLiteral("No report created yet.");
        QString m_lastReportPath;
};
