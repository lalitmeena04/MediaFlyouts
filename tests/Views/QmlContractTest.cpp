#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QRegularExpression>
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
    expectContains(source, QStringLiteral("updateChecker.notifyUpdate()"));
    // A manual check must surface a version the user skipped earlier.
    expectContains(source, QStringLiteral("settings.skippedUpdateVersion = \"\""));
    EXPECT_FALSE(source.contains(QStringLiteral("openReleasePage")));
}

// The update prompt is a dialog. It downloads in-app and then installs.
TEST(QmlContractTest, UpdateDialogSkipsOrDownloadsThenInstalls) {
    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("property UpdateDialog updateDialog"));
    expectContains(app, QStringLiteral("function onUpdateFound(version)"));
    expectContains(app, QStringLiteral("version !== settings.skippedUpdateVersion"));

    const QString dialog = loadSource("Views/UpdateDialog.qml");
    expectContains(dialog, QStringLiteral("settings.skippedUpdateVersion = updateChecker.latestVersion"));
    expectContains(dialog, QStringLiteral("updateChecker.download()"));
    expectContains(dialog, QStringLiteral("updateChecker.install()"));
    expectContains(dialog, QStringLiteral("updateChecker.cancelDownload()"));
    expectContains(dialog, QStringLiteral("updateChecker.downloadProgress"));
    // Release with no installer still has a way out.
    expectContains(dialog, QStringLiteral("updateChecker.openReleasePage()"));
    // Store copies go to the Store. Portable copies go to the release page.
    expectContains(dialog, QStringLiteral("updateChecker.installKind === \"store\""));
    expectContains(dialog, QStringLiteral("updateChecker.installKind === \"portable\""));
    expectContains(dialog, QStringLiteral("updateChecker.openStorePage()"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("updateChecker.installKindLabel"));
}

// First run offers every major feature, including the newer peripheral ones.
TEST(QmlContractTest, OnboardingOffersPeripheralChargerAndUpdateOptions) {
    const QString source = loadSource("Views/OnboardingWindow.qml");

    expectContains(source, QStringLiteral("readonly property int stepCount: 5"));
    expectContains(source, QStringLiteral("visible: root.step === 4"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutEnabled = value"));
    expectContains(source, QStringLiteral("settings.peripheralsWidgetEnabled = value"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutEnabled = value"));
    // Desktops have no battery, so the charger row must explain itself.
    expectContains(source, QStringLiteral("available: capabilities.chargerAvailable"));
    expectContains(source, QStringLiteral("settings.autoCheckUpdates = value"));
}

// Error reports are created on demand and the user posts them by hand.
TEST(QmlContractTest, AboutPageOffersErrorReportsAndTrayOpensIt) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("errorReporter.createReport()"));
    expectContains(dashboard, QStringLiteral("errorReporter.revealReport()"));
    expectContains(dashboard, QStringLiteral("errorReporter.openIssuePage()"));
    expectContains(dashboard, QStringLiteral("Nothing is sent until you post the issue."));
    expectContains(dashboard, QStringLiteral("function showPage(name)"));

    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("function onReportBugRequested()"));
    expectContains(app, QStringLiteral("dashboard.showPage(\"About\")"));
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

// Every glyph in the palette must have a consumer. Unused glyphs rot.
TEST(QmlContractTest, FluentDefinesNoUnusedGlyphs) {
    const QString fluent = loadSource("Views/Fluent.qml");

    QString usage;
    const QDir views(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QStringLiteral("/Views"));
    for(const QString& file : views.entryList({QStringLiteral("*.qml")}, QDir::Files)) {
        if(file != QStringLiteral("Fluent.qml"))
            usage += loadSource(QStringLiteral("Views/%1").arg(file).toLatin1().constData());
    }
    EXPECT_FALSE(usage.isEmpty());

    const QRegularExpression glyph(QStringLiteral("readonly property string (\\w+): \"\\\\u"));
    auto it = glyph.globalMatch(fluent);
    int glyphs = 0;
    while(it.hasNext()) {
        const QString name = it.next().captured(1);
        ++glyphs;
        EXPECT_TRUE(usage.contains(QStringLiteral("Fluent.") + name)) << "Unused Fluent glyph: " << name.toStdString();
    }
    EXPECT_GT(glyphs, 0);
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

// One MonitorMapper feeds every monitor lookup. QML must not keep a second path.
TEST(QmlContractTest, QmlReadsMonitorFactsFromTheSharedMap) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("model: monitors.names"));
    EXPECT_FALSE(dashboard.contains(QStringLiteral("monitorNames()")));

    // Tooltip clamping moved from PeripheralsWidget into the shared WidgetTooltip.
    for(const char* path : {"Views/App.qml", "Views/WidgetTooltip.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("monitors.geometry("));
    }
    for(const char* path : {"Views/App.qml", "Views/WidgetTooltip.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        EXPECT_FALSE(source.contains(QStringLiteral("taskbar.monitorGeometry(")))
            << "Old per-controller monitor lookup still present in " << path;
    }
}

TEST(QmlContractTest, TaskbarWidgetGatesAudioCaptureOnVisibility) {
    const QString source = loadSource("Views/TaskbarWidget.qml");
    // Capture runs only while the visualizer is on screen.
    expectContains(source, QStringLiteral("visualizerLive: root.visible && settings.taskbarVisualizerEnabled"));
    expectContains(source, QStringLiteral("audioCapture.start()"));
    expectContains(source, QStringLiteral("audioCapture.stop()"));
}

// One button pauses every source. It flips to play-all only when the follow-up option is on.
TEST(QmlContractTest, TaskbarWidgetOffersPauseAllWithOptionalPlayAll) {
    const QString widget = loadSource("Views/TaskbarWidget.qml");
    expectContains(widget, QStringLiteral("pauseAllVisible: settings.taskbarWidgetPauseAllEnabled"));
    expectContains(widget, QStringLiteral("showPlayAll: settings.taskbarWidgetPlayAllEnabled && nowPlaying.canResumeAll"));
    expectContains(widget, QStringLiteral("glyph: root.showPlayAll ? Fluent.playCircle : Fluent.pauseCircle"));
    expectContains(widget, QStringLiteral("onClicked: root.showPlayAll ? nowPlaying.playAll() : nowPlaying.pauseAll()"));
    // Exactly one pause-all button exists in the widget.
    EXPECT_EQ(widget.count(QStringLiteral("nowPlaying.pauseAll()")), 1);
    // Button sits at the right edge, past the visualizer boundary. Extra slot widens the widget.
    expectContains(widget, QStringLiteral("id: pauseAllButton"));
    expectContains(widget, QStringLiteral("anchors.right: parent.right"));
    expectContains(widget, QStringLiteral("anchors.rightMargin: 3 + (root.pauseAllVisible ? root.pauseAllSlot : 0)"));
    expectContains(widget, QStringLiteral("naturalWidth: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)"));
    expectContains(widget, QStringLiteral("onWidthChanged: updatePosition()"));
    // Without play-all the button hides once everything is paused. Slot and width stay.
    expectContains(widget, QStringLiteral("pauseAllButtonShown: root.pauseAllVisible"));
    expectContains(widget, QStringLiteral("&& (settings.taskbarWidgetPlayAllEnabled || !nowPlaying.canResumeAll)"));
    expectContains(widget, QStringLiteral("opacity: root.pauseAllButtonShown ? 1 : 0"));
    expectContains(widget, QStringLiteral("enabled: root.pauseAllButtonShown"));
    EXPECT_FALSE(widget.contains(QStringLiteral("root.pauseAllButtonShown ? root.pauseAllSlot")))
        << "Slot must not collapse when the button hides.";

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("settings.taskbarWidgetPauseAllEnabled = value"));
    expectContains(dashboard, QStringLiteral("visible: settings.taskbarWidgetPauseAllEnabled"));
    expectContains(dashboard, QStringLiteral("settings.taskbarWidgetPlayAllEnabled = value"));
}

// Stack layout pages media and peripheral widgets through one taskbar slot.
// Peripheral pages are split per section: 2.4 GHz, Bluetooth, Wired.
TEST(QmlContractTest, WidgetStackPagesMediaAndPeripheralSections) {
    const QString stack = loadSource("Views/WidgetStack.qml");
    expectContains(stack, QStringLiteral("enabled: settings.taskbarWidgetLayout === 1"));
    expectContains(stack, QStringLiteral("list.push(stack.mediaPage)"));
    expectContains(stack, QStringLiteral("list.push(sections[i].label)"));
    expectContains(stack, QStringLiteral("function cycle(step)"));
    // Media page comes first, then one page per peripheral section.
    EXPECT_LT(stack.indexOf(QStringLiteral("list.push(stack.mediaPage)")),
              stack.indexOf(QStringLiteral("list.push(sections[i].label)")));

    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("property WidgetStack widgetStack"));
    EXPECT_EQ(app.count(QStringLiteral("stack: widgetStack")), 2);

    const QString media = loadSource("Views/TaskbarWidget.qml");
    expectContains(media, QStringLiteral("visible: root.eligible && root.pageShown"));
    expectContains(media, QStringLiteral("root.stack.cycle(wheel.angleDelta.y > 0 ? -1 : 1)"));

    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("visible: root.eligible && root.pageShown"));
    expectContains(peripherals, QStringLiteral("pageActive: !root.stacked || root.activeSection !== \"\""));
    expectContains(peripherals, QStringLiteral("return deviceSections.filter(function(section) { return section.label === root.displayedSection })"));
    expectContains(peripherals, QStringLiteral("model: root.shownSections"));
    expectContains(peripherals, QStringLiteral("root.stack.cycle(event.angleDelta.y > 0 ? -1 : 1)"));
    // Stack layout shares the slot. No offset past the media widget.
    expectContains(peripherals, QStringLiteral("if(!root.stacked && root.mediaWidget && root.mediaWidget.visible)"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("model: [\"Side by side\", \"Stack\"]"));
    expectContains(dashboard, QStringLiteral("onActivated: settings.taskbarWidgetLayout = index"));
}

// Stack pages share one frame sized to the largest widget. Content centers in it.
TEST(QmlContractTest, WidgetStackUsesOneFrameSizedToTheLargestWidget) {
    const QString stack = loadSource("Views/WidgetStack.qml");
    expectContains(stack, QStringLiteral("frameWidth: Math.max(mediaWidget ? mediaWidget.naturalWidth : 0,"));
    expectContains(stack, QStringLiteral("peripheralsWidget ? peripheralsWidget.widestSectionWidth : 0)"));
    expectContains(stack, QStringLiteral("frameHeight: Math.max(mediaWidget ? mediaWidget.naturalHeight : 0,"));

    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("width: root.stacked ? root.stack.frameWidth : root.naturalWidth"));
        expectContains(source, QStringLiteral("height: root.stacked ? root.stack.frameHeight : root.naturalHeight"));
        expectContains(source, QStringLiteral("id: content"));
        expectContains(source, QStringLiteral("anchors.centerIn: parent"));
    }

    const QString media = loadSource("Views/TaskbarWidget.qml");
    expectContains(media, QStringLiteral("naturalWidth: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)"));
    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("function sectionWidth(section)"));
    expectContains(peripherals, QStringLiteral("widest = Math.max(widest, sectionWidth(deviceSections[i]))"));
}

// Page changes and media source changes slide and fade. Pages stay visible until they slid out.
TEST(QmlContractTest, WidgetTransitionsSlideAndFade) {
    const QString fx = loadSource("Views/SlideFade.qml");
    expectContains(fx, QStringLiteral("function enter()"));
    expectContains(fx, QStringLiteral("function leave()"));
    expectContains(fx, QStringLiteral("function replace(swap)"));
    expectContains(fx, QStringLiteral("signal finishedLeave()"));
    expectContains(fx, QStringLiteral("to: -fx.direction * fx.distance"));

    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("id: pageFx"));
        expectContains(source, QStringLiteral("direction: root.stack ? root.stack.lastStep : 1"));
        expectContains(source, QStringLiteral("onFinishedLeave: root.pageShown = false"));
        expectContains(source, QStringLiteral("transform: Translate { y: pageFx.offset }"));
        expectContains(source, QStringLiteral("pageFx.enter()"));
        expectContains(source, QStringLiteral("pageFx.leave()"));
    }

    const QString media = loadSource("Views/TaskbarWidget.qml");
    expectContains(media, QStringLiteral("id: sessionFx"));
    expectContains(media, QStringLiteral("onSessionIndexChanged: if(root.visible) sessionFx.enter()"));
    expectContains(media, QStringLiteral("transform: Translate { x: sessionFx.offset }"));
    // Every source switch records its direction first.
    EXPECT_EQ(media.count(QStringLiteral("nowPlaying.cycleSession(")), 1);
    EXPECT_EQ(media.count(QStringLiteral("root.switchSession(")), 2);

    // Section-to-section pages swap content between leave and enter.
    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("pageFx.replace(function() { root.displayedSection = root.activeSection })"));

    expectContains(loadSource("Views/WidgetStack.qml"), QStringLiteral("lastStep = step > 0 ? 1 : -1"));
}

// Every media widget button names itself on hover.
TEST(QmlContractTest, TaskbarWidgetButtonsHaveTooltips) {
    const QString button = loadSource("Views/WidgetButton.qml");
    expectContains(button, QStringLiteral("property string tooltip"));
    expectContains(button, QStringLiteral("property var tip"));
    expectContains(button, QStringLiteral("button.tip.showAt("));
    expectContains(button, QStringLiteral("button.tip.hide()"));
    // Text can change while hovered.
    expectContains(button, QStringLiteral("onTooltipChanged: refreshTip()"));

    const QString widget = loadSource("Views/TaskbarWidget.qml");
    expectContains(widget, QStringLiteral("WidgetTooltip {"));
    expectContains(widget, QStringLiteral("tooltip: \"Previous\""));
    expectContains(widget, QStringLiteral("tooltip: \"Next\""));
    expectContains(widget, QStringLiteral("tooltip: nowPlaying.isPlaying ? \"Pause\" : \"Play\""));
    expectContains(widget, QStringLiteral("tooltip: root.showPlayAll ? \"Play all\" : \"Pause all\""));
    // No bare IconLabel buttons remain. All go through WidgetButton.
    EXPECT_EQ(widget.count(QStringLiteral("WidgetButton {")), 4);
    EXPECT_FALSE(widget.contains(QStringLiteral("IconLabel {")));
}

// Both taskbar widgets share one tooltip window implementation.
TEST(QmlContractTest, TaskbarWidgetsShareTheTooltipWindow) {
    const QString tooltip = loadSource("Views/WidgetTooltip.qml");
    expectContains(tooltip, QStringLiteral("function showAt(globalPoint, text)"));
    expectContains(tooltip, QStringLiteral("function hide()"));
    expectContains(tooltip, QStringLiteral("monitors.geometry(tip.monitor)"));

    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("WidgetTooltip {"));
        EXPECT_FALSE(source.contains(QStringLiteral("id: tipText")))
            << "Inline tooltip window still present in " << path;
    }
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

// Flyouts whose width follows their content must re-center when it changes.
TEST(QmlContractTest, ContentSizedFlyoutsRestackOnWidthChange) {
    const QString app = loadSource("Views/App.qml");
    for(const char* name : {"NowPlayingFlyout nextUpFlyout", "PeripheralFlyout peripheralFlyout"}) {
        const int start = app.indexOf(QString::fromLatin1(name));
        ASSERT_GT(start, 0) << name;
        const int end = app.indexOf(QStringLiteral("\n    }"), start);
        const QString block = app.mid(start, end - start);
        EXPECT_TRUE(block.contains(QStringLiteral("onWidthChanged: app.restack()"))) << name;
    }
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

