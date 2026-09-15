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
    expectContains(source, QStringLiteral("flyoutStack: [flyout, volumeFlyout, lockKeysFlyout, nextUpFlyout, peripheralFlyout, chargerFlyout]"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutPosition"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutMonitor"));
}

TEST(QmlContractTest, RegistersAndStacksTheChargerFlyout) {
    const QString source = loadSource("Views/App.qml");

    expectContains(source, QStringLiteral("property ChargerFlyout chargerFlyout"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutPosition"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutMonitor"));
}

TEST(QmlContractTest, ChargerFlyoutGatesOnCapabilityAndEventSettings) {
    const QString source = loadSource("Views/ChargerFlyout.qml");

    expectContains(source, QStringLiteral("target: powerStatus"));
    expectContains(source, QStringLiteral("function onChargerChanged(pluggedIn)"));
    expectContains(source, QStringLiteral("capabilities.chargerAvailable"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutEnabled"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutOnPlug"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutOnUnplug"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutDuration"));
}

TEST(QmlContractTest, DashboardHidesTheChargerPageOnSystemsWithNoBattery) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("available: capabilities.chargerAvailable"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutEnabled = value"));
}

// A down subsystem must be explained, not silently dropped.
TEST(QmlContractTest, DashboardBannerNamesUnavailableSubsystems) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("id: statusBanner"));
    expectContains(source, QStringLiteral("visible: capabilities.degraded"));
    expectContains(source, QStringLiteral("capabilities.unavailableNames.join(\", \")"));
    // Page content sits below the banner so nothing is covered.
    expectContains(source, QStringLiteral("anchors.top: statusBanner.bottom"));
}

TEST(QmlContractTest, ConnectsTheFlyoutToPeripheralAlertsAndEventSettings) {
    const QString source = loadSource("Views/PeripheralFlyout.qml");

    expectContains(source, QStringLiteral("target: peripheralAlerts"));
    expectContains(source, QStringLiteral("function onAlertsRaised()"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnConnect"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnLowBattery"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutDuration"));
}

// Wired devices stay silent until the user opts in.
TEST(QmlContractTest, WiredAlertsAreGatedByTheirOwnSetting) {
    const QString source = loadSource("Views/PeripheralFlyout.qml");

    expectContains(source, QStringLiteral("connectionKind === \"usb\""));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnWired"));
}

// No transport signal links a dongle to the same peripheral on another radio,
// so hiding a device has to silence it everywhere.
TEST(QmlContractTest, HiddenDevicesAreIgnoredOnEverySurface) {
    expectContains(loadSource("Views/PeripheralFlyout.qml"),
                   QStringLiteral("!settings.peripheralSelected(alert.name)"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("readonly property var shownDevices"));
    expectContains(dashboard, QStringLiteral("excluded.indexOf(device.name) === -1"));
    // The picker itself keeps listing everything, or a hidden device is unreachable.
    expectContains(dashboard, QStringLiteral("model: settings.peripheralsWidgetEnabled ? dash.wirelessDevices : []"));
}

TEST(QmlContractTest, WidgetHidesWiredDevicesUntilEnabled) {
    const QString widget = loadSource("Views/PeripheralsWidget.qml");

    expectContains(widget, QStringLiteral("settings.peripheralsWidgetWiredEnabled"));
    expectContains(widget, QStringLiteral("\"label\": \"Wired\""));

    // Dashboard offers the group toggle plus a row per wired device.
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("settings.peripheralsWidgetWiredEnabled = value"));
    expectContains(dashboard, QStringLiteral("dash.wiredDevices"));
    expectContains(dashboard, QStringLiteral("settings.setPeripheralSelected(modelData.name, value)"));
}

TEST(QmlContractTest, CardUsesAlertCategoryAndTransport) {
    const QString source = loadSource("Views/PeripheralCard.qml");

    expectContains(source, QStringLiteral("card.alert.connectionKind"));
    expectContains(source, QStringLiteral("card.alert.category"));
    expectContains(source, QStringLiteral("DeviceIcons.transport"));
}

TEST(QmlContractTest, DashboardHasTheRequestedPagesInOrder) {
    const QString source = loadSource("Views/Dashboard.qml");
    const QStringList pages{
        QStringLiteral("Home"), QStringLiteral("Media Flyout"), QStringLiteral("Peripheral Flyout"),
        QStringLiteral("Charger Flyout"), QStringLiteral("Media Widget"), QStringLiteral("Peripheral Widget"),
        QStringLiteral("Volume"), QStringLiteral("Lock Keys"), QStringLiteral("App Filtering"),
        QStringLiteral("Additional Settings"), QStringLiteral("About")};

    int previous = -1;
    for(const QString& page : pages) {
        const int current = source.indexOf(QStringLiteral("\"name\": \"") + page + QStringLiteral("\""));
        EXPECT_GT(current, previous) << "Page is absent or out of order: " << page.toStdString();
        previous = current;
    }
    expectContains(source, QStringLiteral("property bool collapsed"));
}

TEST(QmlContractTest, DashboardResetsScrollPositionWhenPageChanges) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("id: pageScroll"));
    expectContains(source, QStringLiteral("onCurrentIndexChanged: pageScroll.contentItem.contentY = 0"));
}

TEST(QmlContractTest, AboutPageUsesTheGitHubBrandMark) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("qrc:/qt/qml/MediaFlyouts/Views/assets/github-mark-light.svg"));
    expectContains(source, QStringLiteral("qrc:/qt/qml/MediaFlyouts/Views/assets/github-mark-dark.svg"));
    expectContains(source, QStringLiteral("https://github.com/scleaverzer0ne/MediaFlyouts-Releases"));
    EXPECT_FALSE(source.contains(QStringLiteral("glyph: Fluent.link")));
}

// An available update must offer a way to get it, not just an announcement.
TEST(QmlContractTest, AboutPageOffersTheDownloadWhenAnUpdateExists) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("visible: updateChecker.updateAvailable"));
    expectContains(source, QStringLiteral("updateChecker.openReleasePage()"));
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
    expectContains(source, QStringLiteral("camera: \"\\uF255\""));
}

TEST(QmlContractTest, CamerasUseTheCameraGlyph) {
    const QString source = loadSource("Views/DeviceIcons.qml");

    expectContains(source, QStringLiteral("cat === \"camera\""));
    expectContains(source, QStringLiteral("Fluent.camera"));
}

// A cable-powered device has no battery. That is not the same as an unknown reading.
TEST(QmlContractTest, WiredDevicesShowNoBatteryInfo) {
    const QString icons = loadSource("Views/DeviceIcons.qml");
    expectContains(icons, QStringLiteral("function hasBattery(connectionKind)"));
    expectContains(icons, QStringLiteral("connectionKind !== \"usb\""));

    // The gauge can be switched off, leaving a plain outline around the icon.
    const QString bring = loadSource("Views/BatteryRing.qml");
    expectContains(bring, QStringLiteral("property bool showGauge"));
    expectContains(bring, QStringLiteral("property color outlineColor"));
    expectContains(bring, QStringLiteral("if(!ring.showGauge)"));

    const QString widget = loadSource("Views/PeripheralsWidget.qml");
    expectContains(widget, QStringLiteral("showGauge: DeviceIcons.hasBattery(modelData.connectionKind)"));
    expectContains(widget, QStringLiteral("if(DeviceIcons.hasBattery(modelData.connectionKind))"));

    expectContains(loadSource("Views/PeripheralCard.qml"),
                   QStringLiteral("showGauge: DeviceIcons.hasBattery(card.alert.connectionKind)"));
}

TEST(QmlContractTest, WiredWidgetToggleIsSeparatedFromWirelessDevices) {
    const QString source = loadSource("Views/Dashboard.qml");
    const int separator = source.indexOf(QStringLiteral("color: theme.border"),
                                         source.indexOf(QStringLiteral("dash.wirelessDevices")));
    const int wiredToggle = source.indexOf(QStringLiteral("settings.peripheralsWidgetWiredEnabled = value"));

    EXPECT_GT(separator, 0);
    EXPECT_GT(wiredToggle, separator) << "Wired toggle must sit below the separator.";
}

// Renaming must not change the identity used for hiding or for alerts.
TEST(QmlContractTest, PeripheralRowRenamesWithoutLosingTheDetectedName) {
    const QString row = loadSource("Views/PeripheralRow.qml");

    expectContains(row, QStringLiteral("required property string deviceName"));
    expectContains(row, QStringLiteral("settings.peripheralNames[row.deviceName] || row.deviceName"));
    expectContains(row, QStringLiteral("settings.setPeripheralDisplayName(row.deviceName, text)"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("delegate: PeripheralRow"));
    expectContains(dashboard, QStringLiteral("settings.setPeripheralSelected(modelData.name, value)"));
}

TEST(QmlContractTest, RenamedDevicesShowTheirNewNameEverywhere) {
    expectContains(loadSource("Views/PeripheralsWidget.qml"),
                   QStringLiteral("settings.peripheralNames[modelData.name] || modelData.name"));
    expectContains(loadSource("Views/PeripheralCard.qml"),
                   QStringLiteral("settings.peripheralNames[card.alert.name] || card.alert.name"));
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
        // The home overview lists wired devices beside the wireless ones.
        expectContains(source, QStringLiteral("\"label\": \"Wired\""));
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

TEST(QmlContractTest, PeekAreaHoldsTheFlyoutOpenWhileHovered) {
    const QString source = loadSource("Views/PeekArea.qml");
    expectContains(source, QStringLiteral("property var flyout"));
    expectContains(source, QStringLiteral("property bool pinned"));
    expectContains(source, QStringLiteral("hoverEnabled: true"));
    expectContains(source, QStringLiteral("onEntered: if(peek.flyout) peek.flyout.stopAutoHide()"));
    expectContains(source, QStringLiteral("onExited: if(peek.flyout && !peek.pinned) peek.flyout.restartAutoHide()"));
}

TEST(QmlContractTest, FlyoutsUsePeekAreaInsteadOfInlineHoverHandling) {
    for(const char* path : {"Views/MediaFlyout.qml", "Views/VolumeFlyout.qml", "Views/NowPlayingFlyout.qml",
                            "Views/PeripheralFlyout.qml", "Views/ChargerFlyout.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("PeekArea {"));
        expectContains(source, QStringLiteral("flyout: root"));
    }
    // Pinned flyouts stay open after the pointer leaves.
    expectContains(loadSource("Views/MediaFlyout.qml"), QStringLiteral("pinned: settings.mediaFlyoutAlwaysDisplay"));
    expectContains(loadSource("Views/VolumeFlyout.qml"), QStringLiteral("pinned: root.expanded"));
}

TEST(QmlContractTest, ScrubAreaReportsTrackPositionAsFraction) {
    const QString source = loadSource("Views/ScrubArea.qml");
    expectContains(source, QStringLiteral("property real trackWidth"));
    expectContains(source, QStringLiteral("signal scrubStarted(real fraction)"));
    expectContains(source, QStringLiteral("signal scrubMoved(real fraction)"));
    expectContains(source, QStringLiteral("signal scrubEnded(real fraction)"));
    // Fraction stays inside the track.
    expectContains(source, QStringLiteral("Math.max(0, Math.min(1, px / scrub.trackWidth))"));
}

TEST(QmlContractTest, DraggableTracksUseScrubAreaWithoutInlineMath) {
    for(const char* path : {"Views/MediaFlyout.qml", "Views/VolumeFlyout.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("ScrubArea {"));
        expectContains(source, QStringLiteral("trackWidth:"));
        EXPECT_FALSE(source.contains(QStringLiteral("function setFromX(")))
            << "Duplicated drag math still present in " << path;
        EXPECT_FALSE(source.contains(QStringLiteral("function fractionAt(")))
            << "Duplicated drag math still present in " << path;
    }
}

TEST(QmlContractTest, SharedAnimationDurationsComeFromAnim) {
    const QString anim = loadSource("Views/Anim.qml");
    expectContains(anim, QStringLiteral("readonly property int moveMs"));
    expectContains(anim, QStringLiteral("readonly property int fillMs"));
    expectContains(anim, QStringLiteral("readonly property int hoverMs"));
    expectContains(loadSource("Views/FlyoutBase.qml"), QStringLiteral("duration: Anim.moveMs"));
    expectContains(loadSource("Views/PeripheralsWidget.qml"), QStringLiteral("duration: Anim.moveMs"));
    expectContains(loadSource("Views/MediaFlyout.qml"), QStringLiteral("duration: Anim.fillMs"));
    expectContains(loadSource("Views/MediaFlyout.qml"), QStringLiteral("duration: Anim.hoverMs"));
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

