#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QProcess>
#include <QStringList>

#include "AppRestarter.h"

TEST(AppRestarterTest, RelaunchArgumentsAppendTheWaitSwitch) {
    const QStringList args = AppRestarter::relaunchArguments({QStringLiteral("--verbose")}, 4242);
    EXPECT_EQ(args, (QStringList{QStringLiteral("--verbose"), QStringLiteral("--wait-for-pid"), QStringLiteral("4242")}));
}

TEST(AppRestarterTest, RelaunchArgumentsDropAnOlderWaitSwitch) {
    // A second restart must not wait for the pid of a process that is long gone.
    const QStringList args = AppRestarter::relaunchArguments(
        {QStringLiteral("--wait-for-pid"), QStringLiteral("17"), QStringLiteral("--x")}, 99);
    EXPECT_EQ(args, (QStringList{QStringLiteral("--x"), QStringLiteral("--wait-for-pid"), QStringLiteral("99")}));
}

TEST(AppRestarterTest, RelaunchArgumentsWithoutAPidAddNothing) {
    EXPECT_EQ(AppRestarter::relaunchArguments({QStringLiteral("--wait-for-pid"), QStringLiteral("17")}, 0), QStringList{});
}

TEST(AppRestarterTest, PredecessorPidParsesTheSwitch) {
    EXPECT_EQ(AppRestarter::predecessorPid({QStringLiteral("app.exe"), QStringLiteral("--wait-for-pid"), QStringLiteral("1234")}), 1234);
    EXPECT_EQ(AppRestarter::predecessorPid({QStringLiteral("app.exe")}), 0);
    EXPECT_EQ(AppRestarter::predecessorPid({QStringLiteral("--wait-for-pid")}), 0);
    EXPECT_EQ(AppRestarter::predecessorPid({QStringLiteral("--wait-for-pid"), QStringLiteral("abc")}), 0);
    EXPECT_EQ(AppRestarter::predecessorPid({QStringLiteral("--wait-for-pid"), QStringLiteral("-5")}), 0);
}

TEST(AppRestarterTest, WaitForExitIgnoresAnInvalidPid) {
    EXPECT_TRUE(AppRestarter::waitForExit(0, 10));
    EXPECT_TRUE(AppRestarter::waitForExit(-1, 10));
}

#ifdef Q_OS_WIN
TEST(AppRestarterTest, WaitForExitReturnsOnceTheProcessIsGone) {
    QProcess proc;
    proc.start(QStringLiteral("cmd.exe"), {QStringLiteral("/c"), QStringLiteral("exit 0")});
    ASSERT_TRUE(proc.waitForStarted(5000));
    const qint64 pid = proc.processId();
    ASSERT_GT(pid, 0);
    EXPECT_TRUE(AppRestarter::waitForExit(pid, 10000));
    proc.waitForFinished(5000);
}

TEST(AppRestarterTest, WaitForExitTimesOutOnARunningProcess) {
    EXPECT_FALSE(AppRestarter::waitForExit(QCoreApplication::applicationPid(), 50));
}
#endif

TEST(AppRestarterTest, StartsWithNoStatus) {
    AppRestarter restarter(nullptr);
    EXPECT_TRUE(restarter.status().isEmpty());
}
