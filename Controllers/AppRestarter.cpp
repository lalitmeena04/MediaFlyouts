






#include "AppRestarter.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QProcess>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "ErrorReporter.h"
#include "Logging.h"

AppRestarter::AppRestarter(ErrorReporter* reporter, QObject* parent) : QObject(parent), m_reporter(reporter) {}

QString AppRestarter::status() const {
    return m_status;
}

QStringList AppRestarter::relaunchArguments(const QStringList& currentArguments, qint64 pid) {
    QStringList args;
    const QString flag = QString::fromLatin1(kWaitForPidArg);
    for(int i = 0; i < currentArguments.size(); ++i) {
        if(currentArguments.at(i) == flag) {
            ++i; // Drop the value too. A second restart must not wait for a dead pid.
            continue;
        }
        args << currentArguments.at(i);
    }
    if(pid > 0)
        args << flag << QString::number(pid);
    return args;
}

qint64 AppRestarter::predecessorPid(const QStringList& arguments) {
    const int at = arguments.indexOf(QString::fromLatin1(kWaitForPidArg));
    if(at < 0 || at + 1 >= arguments.size())
        return 0;
    bool ok = false;
    const qint64 pid = arguments.at(at + 1).toLongLong(&ok);
    return ok && pid > 0 ? pid : 0;
}

bool AppRestarter::waitForExit(qint64 pid, int timeoutMs) {
    if(pid <= 0)
        return true;
#ifdef Q_OS_WIN
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    if(!process) {
        const DWORD error = GetLastError();
        // No such process: it is already gone.
        if(error == ERROR_INVALID_PARAMETER)
            return true;
        TRC_WARN("restart: cannot open pid {} (error {}); not waiting", pid, error);
        return false;
    }
    const DWORD rc = WaitForSingleObject(process, static_cast<DWORD>(qMax(0, timeoutMs)));
    CloseHandle(process);
    return rc == WAIT_OBJECT_0;
#else
    Q_UNUSED(timeoutMs);
    return false;
#endif
}

void AppRestarter::waitForPredecessor(const QStringList& arguments) {
    const qint64 pid = predecessorPid(arguments);
    if(pid <= 0)
        return;
    QElapsedTimer clock;
    clock.start();
    const bool gone = waitForExit(pid, kPredecessorTimeoutMs);
    TRC_INFO("restart: waited {} ms for predecessor pid {} ({})", clock.elapsed(), pid, gone ? "exited" : "still running");
}

void AppRestarter::restart() {
    TRC_INFO("restart: requested");
    if(m_reporter) {
        m_reporter->createReport();
        TRC_INFO("restart: report {} ({})", m_reporter->lastReportPath().toStdString(), m_reporter->status().toStdString());
    }
    mf::flushLogging();

    const QString exe = QCoreApplication::applicationFilePath();
    const QStringList args = relaunchArguments(QCoreApplication::arguments().mid(1), QCoreApplication::applicationPid());
    qint64 newPid = 0;
    if(!QProcess::startDetached(exe, args, QDir::currentPath(), &newPid)) {
        TRC_WARN("restart: cannot start {}", exe.toStdString());
        m_status = QStringLiteral("Couldn't start a new instance. The app keeps running.");
        emit changed();
        return;
    }
    TRC_INFO("restart: started pid {}; quitting", newPid);
    m_status = QStringLiteral("Restarting...");
    emit changed();
    mf::flushLogging();
    // Leave the caller's signal handler before tearing down.
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}
