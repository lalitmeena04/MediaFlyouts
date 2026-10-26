#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class ErrorReporter;

// Restarts the app. Saves an error report first, so the log of the session
// that went wrong is kept. The new process waits for this one to exit before
// it takes the single-instance slot.
class AppRestarter : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)

    public:
        // Command line switch the new instance gets: --wait-for-pid <pid>.
        static constexpr char kWaitForPidArg[] = "--wait-for-pid";
        // How long a new instance waits for the old one. The old one only has
        // to tear down its windows and hooks.
        static constexpr int kPredecessorTimeoutMs = 15000;

        explicit AppRestarter(ErrorReporter* reporter, QObject* parent = nullptr);

        QString status() const;

        // Arguments for the new process: the current ones minus any old
        // --wait-for-pid pair, plus --wait-for-pid <pid>.
        static QStringList relaunchArguments(const QStringList& currentArguments, qint64 pid);
        // The pid named by --wait-for-pid, or 0 when absent or not a number.
        static qint64 predecessorPid(const QStringList& arguments);
        // Blocks until the process is gone. True when it exited or does not
        // exist, false on timeout or when the wait is not possible.
        static bool waitForExit(qint64 pid, int timeoutMs);
        // Called at startup. Waits for the predecessor named on the command line.
        static void waitForPredecessor(const QStringList& arguments);

    public slots:
        // Save a report, start a new instance, quit this one.
        void restart();

    signals:
        void changed();

    private:
        ErrorReporter* m_reporter = nullptr;
        QString m_status;
};
