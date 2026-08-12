#include <QtTest>

#include <QSignalSpy>
#include <QStandardPaths>

#include "Settings.h"

class SettingsQtTest : public QObject {
        Q_OBJECT

    private slots:
        void initTestCase();
        void init();
        void defaults();
        void setterEmitsChanged();
        void redundantSetDoesNotEmit();
        void persistsAcrossInstances();
};

void SettingsQtTest::initTestCase() {
    // keep the tests from touching the real user configuration
    QStandardPaths::setTestModeEnabled(true);
}

void SettingsQtTest::init() {
    QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
              QStringLiteral("MediaFlyouts"))
        .clear();
}

void SettingsQtTest::defaults() {
    Settings s;
    QVERIFY(s.mediaFlyoutEnabled());
    QVERIFY(s.volumeFlyoutEnabled());
    QVERIFY(s.overrideNativeOsd());
    QVERIFY(s.lockKeysEnabled());
    QVERIFY(s.nextUpEnabled());
    QVERIFY(s.disableOnFullscreen());
    QVERIFY(!s.launchOnStartup());
    QVERIFY(!s.taskbarWidgetEnabled());
    QVERIFY(s.taskbarWidgetControlsEnabled());
}

void SettingsQtTest::setterEmitsChanged() {
    Settings s;
    QSignalSpy spy(&s, &Settings::changed);
    s.setNextUpEnabled(false);
    QVERIFY(!s.nextUpEnabled());
    QCOMPARE(spy.count(), 1);
}

void SettingsQtTest::redundantSetDoesNotEmit() {
    Settings s;
    QSignalSpy spy(&s, &Settings::changed);
    s.setNextUpEnabled(true); // already default
    QCOMPARE(spy.count(), 0);
}

void SettingsQtTest::persistsAcrossInstances() {
    {
        Settings first;
        first.setDisableOnFullscreen(false);
    }
    Settings second;
    QVERIFY(!second.disableOnFullscreen());
}

QTEST_GUILESS_MAIN(SettingsQtTest)
#include "SettingsQtTest.moc"
