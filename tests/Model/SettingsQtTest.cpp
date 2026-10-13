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
        void legacySettingsKeepTheirValuesAndGainNewDefaults();
};

void SettingsQtTest::initTestCase() {
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
    QVERIFY(s.seekbarEnabled());
    QVERIFY(s.seekbarInteractive());
    QVERIFY(!s.mediaFlyoutAlwaysDisplay());
    QVERIFY(!s.centerTitleArtist());
    QVERIFY(!s.compactLayout());
    QVERIFY(s.playerInfoEnabled());
    QVERIFY(s.repeatEnabled());
    QVERIFY(s.shuffleEnabled());
    QVERIFY(s.volumeFlyoutEnabled());
    QVERIFY(!s.volumeFlyoutMixerEnabled());
    QVERIFY(s.overrideNativeOsd());
    QVERIFY(s.lockKeysEnabled());
    QVERIFY(s.lockKeysCapsEnabled());
    QVERIFY(s.lockKeysNumEnabled());
    QVERIFY(s.lockKeysScrollEnabled());
    QVERIFY(!s.lockKeysInsertEnabled());
    QVERIFY(s.lockKeysBoldUi());
    QCOMPARE(s.lockKeysDuration(), 2000);
    QVERIFY(s.nextUpEnabled());
    QVERIFY(s.disableOnFullscreen());
    QVERIFY(s.launchOnStartup());
    QVERIFY(!s.trayIconHidden());
    QVERIFY(s.trayLeftClickShowsFlyout());
    QVERIFY(s.taskbarWidgetEnabled());
    QVERIFY(s.taskbarWidgetControlsEnabled());
    QCOMPARE(s.flyoutPosition(), 4);
    QCOMPARE(s.flyoutMonitor(), 0);
    QCOMPARE(s.flyoutDuration(), 4500);
    QCOMPARE(s.flyoutAnimationSpeed(), 2);
    QCOMPARE(s.flyoutAnimationMs(), 300);
    QCOMPARE(s.flyoutAnimationEasingStyle(), 3);
    QCOMPARE(s.volumeFlyoutDuration(), 1800);
    QCOMPARE(s.volumeFlyoutPosition(), 4);
    QCOMPARE(s.volumeFlyoutMonitor(), 0);
    QCOMPARE(s.nextUpDuration(), 2000);
    QCOMPARE(s.nextUpStyle(), 0);
    QCOMPARE(s.taskbarWidgetOffset(), 0);
    QVERIFY(s.taskbarWidgetSide() == 0 || s.taskbarWidgetSide() == 2);
    QCOMPARE(s.taskbarWidgetMonitor(), 0);
    QCOMPARE(s.taskbarWidgetLayout(), 0);
    QVERIFY(s.taskbarWidgetShowPauseOverlay());
    QVERIFY(s.taskbarWidgetScrollingEnabled());
    QVERIFY(!s.taskbarWidgetPauseAllEnabled());
    QVERIFY(s.taskbarWidgetPlayAllEnabled());
    QVERIFY(!s.taskbarVisualizerEnabled());
    QVERIFY(s.taskbarVisualizerCentered());
    QCOMPARE(s.visualizerStyle(), 0);
    QVERIFY(!s.peripheralFlyoutEnabled());
    QCOMPARE(s.peripheralFlyoutDuration(), 3000);
    QCOMPARE(s.peripheralFlyoutPosition(), 4);
    QVERIFY(s.peripheralFlyoutOnConnect());
    QVERIFY(!s.peripheralFlyoutOnCharging());
    QVERIFY(s.peripheralLowBatteryToast());
    QVERIFY(!s.onboardingCompleted());
    QVERIFY(!s.accentFromArt());
    QVERIFY(s.accentFromArtAppWide());
    QCOMPARE(s.accentMode(), 0);
    QCOMPARE(s.customAccent(), QStringLiteral("#4CC2FF"));
    QCOMPARE(s.appTheme(), 0);
    QCOMPARE(s.filterMode(), 0);
    QVERIFY(s.filterList().isEmpty());
    QVERIFY(!s.autoCheckUpdates());
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
    s.setNextUpEnabled(true);
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

void SettingsQtTest::legacySettingsKeepTheirValuesAndGainNewDefaults() {
    QSettings legacy(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                     QStringLiteral("MediaFlyouts"));
    legacy.setValue(QStringLiteral("mediaFlyoutEnabled"), false);
    legacy.setValue(QStringLiteral("flyoutPosition"), 2);
    legacy.setValue(QStringLiteral("taskbarWidgetEnabled"), false);
    legacy.sync();

    Settings settings;
    QVERIFY(!settings.mediaFlyoutEnabled());
    QCOMPARE(settings.flyoutPosition(), 2);
    QVERIFY(!settings.taskbarWidgetEnabled());

    QVERIFY(!settings.peripheralFlyoutEnabled());
    QCOMPARE(settings.peripheralFlyoutDuration(), 3000);
    QVERIFY(settings.peripheralFlyoutOnConnect());
    QVERIFY(settings.peripheralLowBatteryToast());
    QVERIFY(!settings.peripheralFlyoutOnWired());
    QVERIFY(!settings.peripheralsWidgetWiredEnabled());
    QVERIFY(!settings.chargerFlyoutEnabled());
    QVERIFY(settings.chargerFlyoutOnPlug());
    QVERIFY(settings.chargerFlyoutOnUnplug());
    QCOMPARE(settings.chargerFlyoutDuration(), 3000);
}

QTEST_GUILESS_MAIN(SettingsQtTest)
#include "SettingsQtTest.moc"
