#include <gtest/gtest.h>

#include <QFile>
#include <QString>

namespace {

QString loadSource(const char* relativePath) {
    QFile file(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QLatin1Char('/') + QString::fromLatin1(relativePath));
    EXPECT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    return QString::fromUtf8(file.readAll());
}

void expectContains(const QString& source, const QString& expected) {
    EXPECT_TRUE(source.contains(expected)) << "Missing QML contract: " << expected.toStdString();
}

} // namespace

TEST(QmlContractTest, RegistersAndStacksThePeripheralFlyout) {
    const QString source = loadSource("Views/App.qml");

    expectContains(source, QStringLiteral("property PeripheralFlyout peripheralFlyout"));
    expectContains(source, QStringLiteral("flyoutStack: [flyout, volumeFlyout, lockKeysFlyout, nextUpFlyout, peripheralFlyout]"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutPosition"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutMonitor"));
}

TEST(QmlContractTest, ConnectsTheFlyoutToPeripheralAlertsAndEventSettings) {
    const QString source = loadSource("Views/PeripheralFlyout.qml");

    expectContains(source, QStringLiteral("target: peripheralAlerts"));
    expectContains(source, QStringLiteral("function onAlertsRaised()"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnConnect"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnLowBattery"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutDuration"));
}

TEST(QmlContractTest, CardUsesAlertCategoryAndTransport) {
    const QString source = loadSource("Views/PeripheralCard.qml");

    expectContains(source, QStringLiteral("card.alert.connectionKind"));
    expectContains(source, QStringLiteral("card.alert.category"));
    expectContains(source, QStringLiteral("DeviceIcons.transport"));
}

TEST(QmlContractTest, DashboardHasTheRequestedTenPagesInOrder) {
    const QString source = loadSource("Views/Dashboard.qml");
    const QStringList pages{
        QStringLiteral("Home"), QStringLiteral("Media Flyout"), QStringLiteral("Peripheral Flyout"),
        QStringLiteral("Media Widget"), QStringLiteral("Peripheral Widget"), QStringLiteral("Volume"),
        QStringLiteral("Lock Keys"), QStringLiteral("App Filtering"), QStringLiteral("Additional Settings"),
        QStringLiteral("About")};

    int previous = -1;
    for(const QString& page : pages) {
        const int current = source.indexOf(QStringLiteral("\"name\": \"") + page + QStringLiteral("\""));
        EXPECT_GT(current, previous) << "Page is absent or out of order: " << page.toStdString();
        previous = current;
    }
    expectContains(source, QStringLiteral("property bool collapsed"));
}

TEST(QmlContractTest, DeviceIconsUseFluentWithFontAwesomeFallbackForMouse) {
    const QString source = loadSource("Views/DeviceIcons.qml");

    // Fluent covers most device types.
    expectContains(source, QStringLiteral("import MediaFlyouts"));
    expectContains(source, QStringLiteral("Fluent.keyboard"));
    expectContains(source, QStringLiteral("Fluent.gamepad"));
    expectContains(source, QStringLiteral("Fluent.headset"));
    expectContains(source, QStringLiteral("Fluent.headphones"));

    // Mouse uses bundled Font Awesome glyph.
    expectContains(source, QStringLiteral("FontAwesome6-Solid.ttf"));
    expectContains(source, QStringLiteral("\"fa\": true"));
    expectContains(source, QStringLiteral("\\uF8CC")); // Font Awesome computer-mouse
    expectContains(source, QStringLiteral("function family("));
}

TEST(QmlContractTest, FluentDefinesPeripheralGlyphs) {
    const QString source = loadSource("Views/Fluent.qml");

    expectContains(source, QStringLiteral("headphones: \"\\uF476\""));
    expectContains(source, QStringLiteral("headset: \"\\uF478\""));
    expectContains(source, QStringLiteral("gamepad: \"\\uEEB9\""));
    expectContains(source, QStringLiteral("bluetooth: \"\\uE15E\""));
    expectContains(source, QStringLiteral("usbStick: \"\\uF857\""));
}

TEST(QmlContractTest, IconRenderSitesSelectTheResolvedFontFamily) {
    // Each device glyph site must choose the matching font.
    for(const char* path : {"Views/PeripheralsWidget.qml", "Views/PeripheralCard.qml", "Views/Dashboard.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("font.family: DeviceIcons.family("));
    }
}

TEST(QmlContractTest, GroupsPeripheralWidgetsByConnectionKind) {
    for(const char* path : {"Views/PeripheralsWidget.qml", "Views/Dashboard.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("2.4 GHz"));
        expectContains(source, QStringLiteral("Bluetooth"));
        expectContains(source, QStringLiteral("connectionKind === \"wireless\""));
        expectContains(source, QStringLiteral("connectionKind === \"bluetooth\""));
    }

    const QString widget = loadSource("Views/PeripheralsWidget.qml");
    expectContains(widget, QStringLiteral("Fluent.bluetooth"));
    expectContains(widget, QStringLiteral("Fluent.usbStick"));
}

TEST(QmlContractTest, WidgetsHideForEveryFullscreenApp) {
    // DockableWidget hides taskbar widgets for fullscreen apps.
    const QString base = loadSource("Views/DockableWidget.qml");
    expectContains(base, QStringLiteral("fullscreen.isFullscreen(base.dockMonitor)"));
    EXPECT_FALSE(base.contains(QStringLiteral("fullscreen.shouldBlock(")));
    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        EXPECT_FALSE(source.contains(QStringLiteral("fullscreen.shouldBlock(")));
    }
}

TEST(QmlContractTest, FlyoutsGateOnPlatformCapabilities) {
    const struct {
        const char* path;
        const char* token;
    } cases[] = {
        {"Views/VolumeFlyout.qml", "capabilities.audioAvailable"},
        {"Views/MediaFlyout.qml", "capabilities.mediaSessionAvailable"},
        {"Views/LockKeysFlyout.qml", "capabilities.lockKeysAvailable"},
        {"Views/NowPlayingFlyout.qml", "capabilities.mediaSessionAvailable"},
    };
    for(const auto& c : cases) {
        const QString source = loadSource(c.path);
        expectContains(source, QString::fromLatin1(c.token));
    }
}

TEST(QmlContractTest, FlyoutBasePinsWindowTopmostWhenPresented) {
    const QString source = loadSource("Views/FlyoutBase.qml");

    expectContains(source, QStringLiteral("function present()"));
    expectContains(source, QStringLiteral("base.visible = true"));
    expectContains(source, QStringLiteral("windowEffects.pinTopmost(base)"));
    expectContains(source, QStringLiteral("taskbar.pinTopmost(base)"));
}

TEST(QmlContractTest, SettingRowSupportsUnavailableState) {
    const QString source = loadSource("Views/SettingRow.qml");
    expectContains(source, QStringLiteral("property bool available"));
    expectContains(source, QStringLiteral("property string unavailableNote"));
}

TEST(QmlContractTest, FlyoutBaseOwnsSharedWindowChromeAndTiming) {
    const QString base = loadSource("Views/FlyoutBase.qml");
    expectContains(base, QStringLiteral("flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool"));
    expectContains(base, QStringLiteral("function present()"));
    expectContains(base, QStringLiteral("function hideFlyout()"));
    expectContains(base, QStringLiteral("function stopAutoHide()"));
    expectContains(base, QStringLiteral("function restartAutoHide()"));
    expectContains(base, QStringLiteral("property int hideInterval"));
}

TEST(QmlContractTest, FlyoutsDeriveFromFlyoutBaseWithoutDuplicatingChrome) {
    for(const char* path : {"Views/MediaFlyout.qml", "Views/VolumeFlyout.qml", "Views/LockKeysFlyout.qml",
                            "Views/NowPlayingFlyout.qml", "Views/PeripheralFlyout.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("FlyoutBase {"));
        // FlyoutBase owns chrome and timers.
        EXPECT_FALSE(source.contains(QStringLiteral("flags: Qt.FramelessWindowHint")))
            << "Duplicated window chrome still present in " << path;
        EXPECT_FALSE(source.contains(QStringLiteral("id: hideTimer")))
            << "Duplicated hide timer still present in " << path;
    }
}

TEST(QmlContractTest, DockableWidgetOwnsSharedDockingAndBlocking) {
    const QString base = loadSource("Views/DockableWidget.qml");
    expectContains(base, QStringLiteral("flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus"));
    expectContains(base, QStringLiteral("property bool blocked"));
    expectContains(base, QStringLiteral("property bool pollActive"));
    expectContains(base, QStringLiteral("function updatePosition()"));
    expectContains(base, QStringLiteral("fullscreen.isFullscreen(base.dockMonitor)"));
}

TEST(QmlContractTest, TaskbarWidgetsDeriveFromDockableWidgetWithoutDuplicatingChrome) {
    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("DockableWidget {"));
        expectContains(source, QStringLiteral("pollActive:"));
        // DockableWidget owns block state and fullscreen polling.
        // PeripheralsWidget has a tooltip Window with flags.
        EXPECT_FALSE(source.contains(QStringLiteral("property bool blocked")))
            << "Duplicated block state still present in " << path;
        EXPECT_FALSE(source.contains(QStringLiteral("interval: 500")))
            << "Duplicated fullscreen poll timer still present in " << path;
    }
}

TEST(QmlContractTest, TaskbarWidgetGatesAudioCaptureOnVisibility) {
    const QString source = loadSource("Views/TaskbarWidget.qml");
    // Capture runs only while the visualizer is on screen.
    expectContains(source, QStringLiteral("visualizerLive: root.visible && settings.taskbarVisualizerEnabled"));
    expectContains(source, QStringLiteral("audioCapture.start()"));
    expectContains(source, QStringLiteral("audioCapture.stop()"));
}

TEST(QmlContractTest, PeripheralWidgetPageUsesVerticalBatteryIcon) {
    const QString source = loadSource("Views/Dashboard.qml");
    expectContains(source, QStringLiteral("BatteryIcon"));
    // Fill follows the battery percent.
    expectContains(source, QStringLiteral("modelData.percent / 100"));
}

TEST(QmlContractTest, BundlesTheFontAwesomeFallbackFont) {
    QFile font(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QStringLiteral("/Views/assets/fonts/FontAwesome6-Solid.ttf"));
    EXPECT_TRUE(font.exists()) << "Font Awesome fallback font is missing from the assets";
    EXPECT_GT(font.size(), 0);
}

