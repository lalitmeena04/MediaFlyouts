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

TEST(QmlContractTest, DashboardBannerNamesUnavailableSubsystems) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("id: statusBanner"));
    expectContains(source, QStringLiteral("visible: capabilities.degraded"));
    expectContains(source, QStringLiteral("capabilities.unavailableNames.join(\", \")"));
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

TEST(QmlContractTest, WiredAlertsAreGatedByTheirOwnSetting) {
    const QString source = loadSource("Views/PeripheralFlyout.qml");

    expectContains(source, QStringLiteral("connectionKind === \"usb\""));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutOnWired"));
}

TEST(QmlContractTest, HiddenDevicesAreIgnoredOnEverySurface) {
    expectContains(loadSource("Views/PeripheralFlyout.qml"),
                   QStringLiteral("!settings.peripheralSelected(alert.name)"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("readonly property var shownDevices"));
    expectContains(dashboard, QStringLiteral("excluded.indexOf(device.name) === -1"));
    expectContains(dashboard, QStringLiteral("model: settings.peripheralsWidgetEnabled ? dash.wirelessDevices : []"));
}

TEST(QmlContractTest, WidgetHidesWiredDevicesUntilEnabled) {
    const QString widget = loadSource("Views/PeripheralsWidget.qml");

    expectContains(widget, QStringLiteral("settings.peripheralsWidgetWiredEnabled"));
    expectContains(widget, QStringLiteral("\"label\": qsTr(\"Wired\")"));

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
        const int current = source.indexOf(QStringLiteral("\"name\": qsTr(\"") + page + QStringLiteral("\")"));
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
    expectContains(source, QStringLiteral("https://github.com/lalitmeena04/MediaFlyouts-Releases"));
    EXPECT_FALSE(source.contains(QStringLiteral("glyph: Fluent.link")));
}

TEST(QmlContractTest, AppLogoRendersTheBundledIconBitmap) {
    const QString logo = loadSource("Views/AppLogo.qml");
    expectContains(logo, QStringLiteral("source: \"qrc:/icons/icon-256.png\""));
    expectContains(logo, QStringLiteral("mipmap: true"));
    expectContains(logo, QStringLiteral("property int size"));
}

TEST(QmlContractTest, EveryWindowHeaderShowsTheAppLogo) {
    // The music-note glyph stays for album-art placeholders only.
    const QRegularExpression accentGlyph(
        QStringLiteral("glyph: Fluent\\.musicNote\\s*\\n\\s*size: \\d+\\s*\\n\\s*color: theme\\.accent"));
    for(const char* path : {"Views/Dashboard.qml", "Views/OnboardingWindow.qml", "Views/UpdateDialog.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("AppLogo {"));
        EXPECT_FALSE(source.contains(accentGlyph)) << path << " still uses the note glyph as the app mark";
    }
}

TEST(QmlContractTest, AboutPageOffersTheDownloadWhenAnUpdateExists) {
    const QString source = loadSource("Views/Dashboard.qml");

    expectContains(source, QStringLiteral("visible: updateChecker.updateAvailable"));
    expectContains(source, QStringLiteral("updateChecker.notifyUpdate()"));
    expectContains(source, QStringLiteral("settings.skippedUpdateVersion = \"\""));
    EXPECT_FALSE(source.contains(QStringLiteral("openReleasePage")));
}

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
    expectContains(dialog, QStringLiteral("updateChecker.openReleasePage()"));
    expectContains(dialog, QStringLiteral("updateChecker.installKind === \"store\""));
    expectContains(dialog, QStringLiteral("updateChecker.installKind === \"portable\""));
    expectContains(dialog, QStringLiteral("updateChecker.openStorePage()"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("updateChecker.installKindLabel"));
}

TEST(QmlContractTest, OnboardingOffersPeripheralChargerAndUpdateOptions) {
    const QString source = loadSource("Views/OnboardingWindow.qml");

    expectContains(source, QStringLiteral("readonly property int stepCount: 5"));
    expectContains(source, QStringLiteral("visible: root.step === 4"));
    expectContains(source, QStringLiteral("settings.peripheralFlyoutEnabled = value"));
    expectContains(source, QStringLiteral("settings.peripheralsWidgetEnabled = value"));
    expectContains(source, QStringLiteral("settings.chargerFlyoutEnabled = value"));
    expectContains(source, QStringLiteral("available: capabilities.chargerAvailable"));
    expectContains(source, QStringLiteral("settings.autoCheckUpdates = value"));
}

TEST(QmlContractTest, AboutPageOffersErrorReportsAndTrayOpensIt) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("errorReporter.createReport()"));
    expectContains(dashboard, QStringLiteral("errorReporter.revealReport()"));
    expectContains(dashboard, QStringLiteral("errorReporter.openIssuePage()"));
    expectContains(dashboard, QStringLiteral("The issue page opens with a blank template."));
    expectContains(dashboard, QStringLiteral("Nothing is sent until you post the issue."));
    expectContains(dashboard, QStringLiteral("function showPage(key)"));

    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("function onReportBugRequested()"));
    expectContains(app, QStringLiteral("dashboard.showPage(\"about\")"));
}

TEST(QmlContractTest, PagesAreLookedUpByStableKeyNotTranslatedName) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("if(pages[i].key === key)"));
    expectContains(dashboard, QStringLiteral("\"key\": \"about\""));
    expectContains(dashboard, QStringLiteral("\"key\": \"home\""));
    // Page names are translated, so they are not stable identifiers.
    expectContains(dashboard, QStringLiteral("\"name\": qsTr(\"About\")"));
}

TEST(QmlContractTest, LanguagePickerBindsToTranslatorAndSettings) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("id: languageCombo"));
    expectContains(dashboard, QStringLiteral("i18n.languages.map(function(entry) { return entry.code })"));
    expectContains(dashboard, QStringLiteral("entry.code === \"\" ? qsTr(\"System default\") : entry.name"));
    expectContains(dashboard, QStringLiteral("languageCodes.indexOf(settings.language)"));
    expectContains(dashboard, QStringLiteral("onActivated: settings.language = languageCodes[index]"));

    const QString onboarding = loadSource("Views/OnboardingWindow.qml");
    expectContains(onboarding, QStringLiteral("text: qsTr(\"Choose a language and theme\")"));
    expectContains(onboarding, QStringLiteral("entry.code === \"\" ? qsTr(\"System default\") : entry.name"));
    expectContains(onboarding, QStringLiteral("onActivated: settings.language = languageCodes[index]"));
    // The picker sits on the first real step, before the overlay choices.
    EXPECT_LT(onboarding.indexOf(QStringLiteral("settings.language = languageCodes[index]")),
              onboarding.indexOf(QStringLiteral("visible: root.step === 2")));
}

TEST(QmlContractTest, TranslatedTextIsComposedWithArgNotConcatenation) {
    // "qsTr("Version ") + x" cannot be reordered by a translator. Use qsTr("Version %1").arg(x).
    static const QRegularExpression concat(
        QStringLiteral("qsTr\\(\"(?:[^\"\\\\]|\\\\.)*\"\\)\\s*\\+|\\+\\s*qsTr\\("));
    const QDir views(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/Views"));
    for(const QString& file : views.entryList({QStringLiteral("*.qml")}, QDir::Files)) {
        const QString source = loadSource(QByteArray("Views/" + file.toUtf8()).constData());
        const QRegularExpressionMatch m = concat.match(source);
        EXPECT_FALSE(m.hasMatch()) << file.toStdString() << ": " << m.captured(0).toStdString();
    }
}

TEST(QmlContractTest, UserFacingTextIsWrappedInQsTr) {
    // Plain string literals on text properties bypass the translator. Only the brand name may stay raw.
    static const QRegularExpression rawText(QStringLiteral("^/s*(text|label|title|description|placeholderText|tooltip|"
                                                           "unavailableNote|valueText)/s*:/s*\"([^\"]*)\""),
                                            QRegularExpression::MultilineOption);
    const QDir views(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/Views"));
    const QStringList files = views.entryList({QStringLiteral("*.qml")}, QDir::Files);
    ASSERT_FALSE(files.isEmpty());
    for(const QString& file : files) {
        const QString source = loadSource(QByteArray("Views/" + file.toUtf8()).constData());
        auto it = rawText.globalMatch(source);
        while(it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            const QString literal = m.captured(2);
            const bool hasLetters = literal.contains(QRegularExpression(QStringLiteral("[A-Za-z]")));
            EXPECT_TRUE(!hasLetters || literal == QStringLiteral("MediaFlyouts"))
                << file.toStdString() << ": untranslated literal " << m.captured(0).trimmed().toStdString();
        }
    }
}

TEST(QmlContractTest, AboutPageAndTrayCanRestartTheAppWithAReport) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("restarter.restart()"));
    expectContains(dashboard, QStringLiteral("Save report and restart"));
    expectContains(dashboard, QStringLiteral("text: restarter.status"));
    // The report comes first, so the session log survives the restart.
    EXPECT_LT(dashboard.indexOf(QStringLiteral("errorReporter.createReport()")),
              dashboard.indexOf(QStringLiteral("restarter.restart()")));

    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("function onRestartRequested()"));
    expectContains(app, QStringLiteral("restarter.restart()"));
}

TEST(QmlContractTest, DeviceIconsUseFluentWithFontAwesomeFallbackForMouse) {
    const QString source = loadSource("Views/DeviceIcons.qml");

    expectContains(source, QStringLiteral("import MediaFlyouts"));
    expectContains(source, QStringLiteral("Fluent.keyboard"));
    expectContains(source, QStringLiteral("Fluent.gamepad"));
    expectContains(source, QStringLiteral("Fluent.headset"));
    expectContains(source, QStringLiteral("Fluent.headphones"));

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

TEST(QmlContractTest, WiredDevicesShowNoBatteryInfo) {
    const QString icons = loadSource("Views/DeviceIcons.qml");
    expectContains(icons, QStringLiteral("function hasBattery(connectionKind, percent)"));
    expectContains(icons, QStringLiteral("connectionKind !== \"usb\" || percent >= 0"));

    const QString bring = loadSource("Views/BatteryRing.qml");
    expectContains(bring, QStringLiteral("property bool showGauge"));
    expectContains(bring, QStringLiteral("property color outlineColor"));
    expectContains(bring, QStringLiteral("if(!ring.showGauge)"));

    const QString widget = loadSource("Views/PeripheralsWidget.qml");
    expectContains(widget,
                   QStringLiteral("showGauge: DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent)"));
    expectContains(widget,
                   QStringLiteral("if(DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent))"));

    expectContains(loadSource("Views/PeripheralCard.qml"),
                   QStringLiteral("showGauge: DeviceIcons.hasBattery(card.alert.connectionKind, card.alert.percent)"));
    expectContains(loadSource("Views/Dashboard.qml"),
                   QStringLiteral("DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent)"));
}

TEST(QmlContractTest, BatteryRingShowsChargingSweepAndBadge) {
    const QString ring = loadSource("Views/BatteryRing.qml");
    expectContains(ring, QStringLiteral("property bool charging: false"));
    expectContains(ring, QStringLiteral("property color badgeBackground"));
    expectContains(ring, QStringLiteral("readonly property bool chargingShown: charging && showGauge"));
    expectContains(ring, QStringLiteral("NumberAnimation on sweep"));
    expectContains(ring, QStringLiteral("running: ring.chargingShown && ring.visible"));
    expectContains(ring, QStringLiteral("loops: Animation.Infinite"));
    expectContains(ring, QStringLiteral("id: chargeBadge"));
    expectContains(ring, QStringLiteral("visible: ring.chargingShown"));
    expectContains(ring, QStringLiteral("anchors.verticalCenter: parent.bottom"));
    expectContains(ring, QStringLiteral("text: Fluent.flash"));

    expectContains(loadSource("Views/PeripheralsWidget.qml"), QStringLiteral("charging: modelData.charging"));
    expectContains(loadSource("Views/Dashboard.qml"), QStringLiteral("charging: modelData.charging"));
    const QString card = loadSource("Views/PeripheralCard.qml");
    expectContains(card, QStringLiteral("charging: card.alert.charging === true"));
    expectContains(card, QStringLiteral("badgeBackground: theme.flyout"));
}

TEST(QmlContractTest, WiredWidgetToggleIsSeparatedFromWirelessDevices) {
    const QString source = loadSource("Views/Dashboard.qml");
    const int separator = source.indexOf(QStringLiteral("color: theme.border"),
                                         source.indexOf(QStringLiteral("dash.wirelessDevices")));
    const int wiredToggle = source.indexOf(QStringLiteral("settings.peripheralsWidgetWiredEnabled = value"));

    EXPECT_GT(separator, 0);
    EXPECT_GT(wiredToggle, separator) << "Wired toggle must sit below the separator.";
}

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
        expectContains(source, QStringLiteral("\"label\": qsTr(\"Wired\")"));
    }

    const QString widget = loadSource("Views/PeripheralsWidget.qml");
    expectContains(widget, QStringLiteral("Fluent.bluetooth"));
    expectContains(widget, QStringLiteral("Fluent.usbStick"));
}

TEST(QmlContractTest, WidgetsHideForEveryFullscreenApp) {
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

TEST(QmlContractTest, WindowsRelayoutWhenTheMonitorLayoutChanges) {
    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("target: monitors"));
    expectContains(app, QStringLiteral("monitorConnection"));
    EXPECT_TRUE(app.indexOf(QStringLiteral("app.restack()"), app.indexOf(QStringLiteral("target: monitors"))) > 0)
        << "App.qml must restack flyouts on monitors.changed";

    const QString widget = loadSource("Views/DockableWidget.qml");
    expectContains(widget, QStringLiteral("target: monitors"));
    EXPECT_TRUE(widget.indexOf(QStringLiteral("base.updatePosition()"),
                               widget.indexOf(QStringLiteral("target: monitors"))) > 0)
        << "DockableWidget.qml must re-dock on monitors.changed";
}

TEST(QmlContractTest, TaskbarWidgetsDeriveFromDockableWidgetWithoutDuplicatingChrome) {
    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("DockableWidget {"));
        expectContains(source, QStringLiteral("pollActive:"));
        EXPECT_FALSE(source.contains(QStringLiteral("property bool blocked")))
            << "Duplicated block state still present in " << path;
        EXPECT_FALSE(source.contains(QStringLiteral("interval: 500")))
            << "Duplicated fullscreen poll timer still present in " << path;
    }
}

TEST(QmlContractTest, QmlReadsMonitorFactsFromTheSharedMap) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("model: monitors.names"));
    EXPECT_FALSE(dashboard.contains(QStringLiteral("monitorNames()")));

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
    expectContains(source, QStringLiteral("visualizerLive: root.visible && pro.visualizerEnabled"));
    expectContains(source, QStringLiteral("audioCapture.start()"));
    expectContains(source, QStringLiteral("audioCapture.stop()"));
}

TEST(QmlContractTest, TaskbarWidgetOffersPauseAllWithOptionalPlayAll) {
    const QString widget = loadSource("Views/TaskbarWidget.qml");
    expectContains(widget, QStringLiteral("pauseAllVisible: pro.taskbarWidgetPauseAllEnabled"));
    expectContains(widget, QStringLiteral("showPlayAll: pro.taskbarWidgetPlayAllEnabled && nowPlaying.canResumeAll"));
    expectContains(widget, QStringLiteral("glyph: root.showPlayAll ? Fluent.playCircle : Fluent.pauseCircle"));
    expectContains(widget, QStringLiteral("onClicked: root.showPlayAll ? nowPlaying.playAll() : nowPlaying.pauseAll()"));
    EXPECT_EQ(widget.count(QStringLiteral("nowPlaying.pauseAll()")), 1);
    expectContains(widget, QStringLiteral("id: pauseAllButton"));
    expectContains(widget, QStringLiteral("anchors.right: parent.right"));
    expectContains(widget, QStringLiteral("anchors.rightMargin: 3 + (root.pauseAllVisible ? root.pauseAllSlot : 0)"));
    expectContains(widget, QStringLiteral("naturalWidth: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)"));
    expectContains(widget, QStringLiteral("pauseAllButtonShown: root.pauseAllVisible"));
    expectContains(widget, QStringLiteral("&& (pro.taskbarWidgetPlayAllEnabled || !nowPlaying.canResumeAll)"));
    expectContains(widget, QStringLiteral("opacity: root.pauseAllButtonShown ? 1 : 0"));
    expectContains(widget, QStringLiteral("enabled: root.pauseAllButtonShown"));
    EXPECT_FALSE(widget.contains(QStringLiteral("root.pauseAllButtonShown ? root.pauseAllSlot")))
        << "Slot must not collapse when the button hides.";

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("settings.taskbarWidgetPauseAllEnabled = value"));
    expectContains(dashboard, QStringLiteral("visible: settings.taskbarWidgetPauseAllEnabled"));
    expectContains(dashboard, QStringLiteral("settings.taskbarWidgetPlayAllEnabled = value"));
}

TEST(QmlContractTest, WidgetStackPagesMediaAndPeripheralSections) {
    const QString stack = loadSource("Views/WidgetStack.qml");
    expectContains(stack, QStringLiteral("enabled: pro.taskbarWidgetLayout === 1"));
    expectContains(stack, QStringLiteral("list.push(stack.mediaPage)"));
    expectContains(stack, QStringLiteral("list.push(sections[i].label)"));
    expectContains(stack, QStringLiteral("function cycle(step)"));
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
    expectContains(peripherals, QStringLiteral("if(!root.stacked && root.mediaWidget && root.mediaWidget.visible)"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("model: [qsTr(\"Side by side\"), qsTr(\"Stack\")]"));
    expectContains(dashboard, QStringLiteral("onActivated: settings.taskbarWidgetLayout = index"));
}

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
    EXPECT_EQ(media.count(QStringLiteral("nowPlaying.cycleSession(")), 1);
    EXPECT_EQ(media.count(QStringLiteral("root.switchSession(")), 2);

    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("pageFx.replace(function() { root.displayedSection = root.activeSection })"));

    expectContains(loadSource("Views/WidgetStack.qml"), QStringLiteral("lastStep = step > 0 ? 1 : -1"));
}

TEST(QmlContractTest, TaskbarWidgetButtonsHaveTooltips) {
    const QString button = loadSource("Views/WidgetButton.qml");
    expectContains(button, QStringLiteral("property string tooltip"));
    expectContains(button, QStringLiteral("property var tip"));
    expectContains(button, QStringLiteral("button.tip.showAt("));
    expectContains(button, QStringLiteral("button.tip.hide()"));
    expectContains(button, QStringLiteral("onTooltipChanged: refreshTip()"));

    const QString widget = loadSource("Views/TaskbarWidget.qml");
    expectContains(widget, QStringLiteral("WidgetTooltip {"));
    expectContains(widget, QStringLiteral("tooltip: qsTr(\"Previous\")"));
    expectContains(widget, QStringLiteral("tooltip: qsTr(\"Next\")"));
    expectContains(widget, QStringLiteral("tooltip: nowPlaying.isPlaying ? qsTr(\"Pause\") : qsTr(\"Play\")"));
    expectContains(widget, QStringLiteral("tooltip: root.showPlayAll ? qsTr(\"Play all\") : qsTr(\"Pause all\")"));
    EXPECT_EQ(widget.count(QStringLiteral("WidgetButton {")), 4);
    EXPECT_FALSE(widget.contains(QStringLiteral("IconLabel {")));
}

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
    expectContains(source, QStringLiteral("onExited: if(peek.flyout && !peek.pinned && !peek.swiped) peek.flyout.restartAutoHide()"));
}

TEST(QmlContractTest, PeekAreaRecognizesSwipeAndSkipsClick) {
    const QString source = loadSource("Views/PeekArea.qml");
    expectContains(source, QStringLiteral("property bool swipeEnabled: settings.flyoutSwipeDismissEnabled"));
    expectContains(source, QStringLiteral("peek.mapToGlobal(mouse.x, mouse.y)"));
    EXPECT_FALSE(source.contains(QStringLiteral("mapToItem(null")))
        << "Window-relative coordinates shift while the window is dragged";
    expectContains(source, QStringLiteral("peek.flyout.dragSwipe(d.x, d.y)"));
    expectContains(source, QStringLiteral("peek.swiped = peek.flyout.releaseSwipe(d.x, d.y, Date.now() - peek.pressTime) !== 0"));
    EXPECT_FALSE(source.contains(QStringLiteral("swipe.recognize("))) << "Recognition belongs to FlyoutBase";
    EXPECT_FALSE(source.contains(QStringLiteral("swipe.lockAxis("))) << "Axis lock belongs to FlyoutBase";
    expectContains(source, QStringLiteral("onClicked: if(peek.flyout && !peek.pinned && !peek.swiped) peek.flyout.restartAutoHide()"));
    expectContains(source, QStringLiteral("onCanceled:"));
    expectContains(source, QStringLiteral("peek.flyout.releaseSwipe(0, 0, 0)"));
}

TEST(QmlContractTest, FlyoutBaseFlingsTheWindowOnSwipe) {
    const QString base = loadSource("Views/FlyoutBase.qml");
    expectContains(base, QStringLiteral("default property alias content: slot.data"));
    expectContains(base, QStringLiteral("property real homeX"));
    expectContains(base, QStringLiteral("property real homeY"));
    expectContains(base, QStringLiteral("x: base.homeX + base.swipeDx"));
    expectContains(base, QStringLiteral("y: base.homeY + base.swipeDy"));
    expectContains(base, QStringLiteral("function placeAt(px, py)"));
    expectContains(base, QStringLiteral("property int swipeAxis"));
    expectContains(base, QStringLiteral("swipe.lockAxis(dx, dy, base.swipeAxis)"));
    expectContains(base, QStringLiteral("swipe.project(dx, dy, base.swipeAxis)"));
    expectContains(base, QStringLiteral("xScale: base.swipeScale"));
    expectContains(base, QStringLiteral("function dragSwipe(dx, dy)"));
    expectContains(base, QStringLiteral("function releaseSwipe(dx, dy, elapsedMs)"));
    expectContains(base, QStringLiteral("function dismissBySwipe(direction)"));
    expectContains(base, QStringLiteral("function resetSwipe()"));
    expectContains(base, QStringLiteral("swipe.vector(direction)"));
    EXPECT_FALSE(base.contains(QStringLiteral("base.x = ")));
    EXPECT_FALSE(base.contains(QStringLiteral("base.y = ")));
    EXPECT_FALSE(base.contains(QStringLiteral("Translate {")));
    expectContains(base, QStringLiteral("enabled: base.positioned && !base.swiping"));
    const int dismiss = base.indexOf(QStringLiteral("function dismissBySwipe(direction)"));
    const int dismissEnd = base.indexOf(QStringLiteral("\n    }"), dismiss);
    const QString body = base.mid(dismiss, dismissEnd - dismiss);
    EXPECT_TRUE(body.contains(QStringLiteral("hideTimer.stop()")));
    EXPECT_TRUE(body.contains(QStringLiteral("fadeOut.restart()")));
    const int fade = base.indexOf(QStringLiteral("id: fadeOut"));
    EXPECT_GT(base.indexOf(QStringLiteral("base.resetSwipe()"), fade), fade);
    for(const char* path : {"Views/MediaFlyout.qml", "Views/VolumeFlyout.qml", "Views/LockKeysFlyout.qml",
                            "Views/NowPlayingFlyout.qml", "Views/PeripheralFlyout.qml", "Views/ChargerFlyout.qml"}) {
        EXPECT_FALSE(loadSource(path).contains(QStringLiteral("swipe."))) << path;
    }
}

TEST(QmlContractTest, AppStacksFlyoutsByHomePosition) {
    const QString app = loadSource("Views/App.qml");
    EXPECT_EQ(app.count(QStringLiteral("f.placeAt(")), 2);
    EXPECT_FALSE(app.contains(QStringLiteral("f.x = ")));
    EXPECT_FALSE(app.contains(QStringLiteral("f.y = ")));
}

TEST(QmlContractTest, DashboardExposesSwipeDismissSetting) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("value: settings.flyoutSwipeDismissEnabled"));
    expectContains(dashboard, QStringLiteral("onToggled: settings.flyoutSwipeDismissEnabled = value"));
}

TEST(QmlContractTest, FlyoutsUsePeekAreaInsteadOfInlineHoverHandling) {
    for(const char* path : {"Views/MediaFlyout.qml", "Views/VolumeFlyout.qml", "Views/NowPlayingFlyout.qml",
                            "Views/PeripheralFlyout.qml", "Views/ChargerFlyout.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("PeekArea {"));
        expectContains(source, QStringLiteral("flyout: root"));
    }
    expectContains(loadSource("Views/MediaFlyout.qml"), QStringLiteral("pinned: settings.mediaFlyoutAlwaysDisplay"));
    expectContains(loadSource("Views/VolumeFlyout.qml"), QStringLiteral("pinned: root.expanded"));
}

TEST(QmlContractTest, AllFlyoutsRestackOnSizeChange) {
    const QString app = loadSource("Views/App.qml");
    for(const char* name : {"MediaFlyout flyout", "VolumeFlyout volumeFlyout", "LockKeysFlyout lockKeysFlyout",
                            "NowPlayingFlyout nextUpFlyout", "PeripheralFlyout peripheralFlyout",
                            "ChargerFlyout chargerFlyout"}) {
        const int start = app.indexOf(QString::fromLatin1(name));
        ASSERT_GT(start, 0) << name;
        const int end = app.indexOf(QStringLiteral("\n    }"), start);
        const QString block = app.mid(start, end - start);
        EXPECT_TRUE(block.contains(QStringLiteral("onWidthChanged: app.restack()"))) << name;
        EXPECT_TRUE(block.contains(QStringLiteral("onHeightChanged: app.restack()"))) << name;
    }
}

TEST(QmlContractTest, SettingComboKeepsSelectionWhenTheModelIsReplaced) {
    const QString combo = loadSource("Views/SettingCombo.qml");
    expectContains(combo, QStringLiteral("onModelChanged: currentIndex = Qt.binding(function() { return root.currentIndex })"));
    const QString dashboard = loadSource("Views/Dashboard.qml");
    EXPECT_GE(dashboard.count(QStringLiteral("model: monitors.names")), 5);
}

TEST(QmlContractTest, DockableWidgetRedocksOnSizeChange) {
    const QString base = loadSource("Views/DockableWidget.qml");
    expectContains(base, QStringLiteral("onWidthChanged: base.updatePosition()"));
    expectContains(base, QStringLiteral("onHeightChanged: base.updatePosition()"));
    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString widget = loadSource(path);
        EXPECT_FALSE(widget.contains(QStringLiteral("onWidthChanged: updatePosition()")))
            << "Duplicated redock handler in " << path;
    }
}

TEST(QmlContractTest, DockableWidgetOwnsTheBackdropAwarePalette) {
    const QString base = loadSource("Views/DockableWidget.qml");
    expectContains(base, QStringLiteral("adaptiveColors: pro.taskbarWidgetAdaptiveColors"));
    expectContains(base, QStringLiteral("lightBackdrop: base.adaptiveColors && taskbarBackdrop.light"));
    expectContains(base, QStringLiteral("running: base.visible && base.adaptiveColors"));
    expectContains(base, QStringLiteral("taskbarBackdrop.sample(base)"));
    expectContains(base, QStringLiteral("taskbarBackdrop.readable(theme.accent, taskbarBackdrop.light)"));
    for(const char* name : {"fgPrimary", "fgSecondary", "fgControl", "fgMuted", "hoverBg", "artPlaceholder",
                            "divider", "accent"}) {
        expectContains(base, QStringLiteral("readonly property color %1:").arg(QLatin1String(name)));
    }
}

TEST(QmlContractTest, TaskbarWidgetsUseThePaletteInsteadOfFixedColors) {
    const QString media = loadSource("Views/TaskbarWidget.qml");
    expectContains(media, QStringLiteral("color: root.fgPrimary"));
    expectContains(media, QStringLiteral("color: root.fgSecondary"));
    expectContains(media, QStringLiteral("idleColor: root.fgControl"));
    expectContains(media, QStringLiteral("accent: root.accent"));
    EXPECT_FALSE(media.contains(QStringLiteral("color: \"#AAAAAA\"")));
    EXPECT_FALSE(media.contains(QStringLiteral("idleColor: \"white\"")));
    EXPECT_FALSE(media.contains(QStringLiteral("color: \"#333333\"")));

    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("color: root.accent"));
    expectContains(peripherals, QStringLiteral("color: root.divider"));
    EXPECT_FALSE(peripherals.contains(QStringLiteral("color: theme.accent")));

    for(const char* path : {"Views/TaskbarWidget.qml", "Views/PeripheralsWidget.qml"}) {
        const QString source = loadSource(path);
        expectContains(source, QStringLiteral("? root.hoverBg : \"transparent\""));
        EXPECT_FALSE(source.contains(QStringLiteral("? \"#2A2A2A\" : \"transparent\"")))
            << "Fixed hover color still present in " << path;
    }

    const QString visualizer = loadSource("Views/TaskbarVisualizer.qml");
    expectContains(visualizer, QStringLiteral("property color accent: theme.accent"));
    expectContains(visualizer, QStringLiteral(": viz.accent"));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("settings.taskbarWidgetAdaptiveColors = value"));
}

TEST(QmlContractTest, DashboardOffersAConfirmedResetToDefaults) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("{ \"label\": qsTr(\"Restore default settings\"), \"page\": 9 }"));
    expectContains(dashboard, QStringLiteral("text: qsTr(\"Restore default settings…\")"));
    expectContains(dashboard, QStringLiteral("onClicked: resetConfirm.open()"));
    expectContains(dashboard, QStringLiteral("id: resetConfirm"));
    expectContains(dashboard, QStringLiteral("modal: true"));
    expectContains(dashboard, QStringLiteral("settings.resetToDefaults()"));
    // The reset must only run from the confirmation popup, never straight from the page button.
    EXPECT_EQ(dashboard.count(QStringLiteral("settings.resetToDefaults()")), 1);
    expectContains(dashboard, QStringLiteral("function resetToDefaults()"));
    EXPECT_EQ(dashboard.count(QStringLiteral("dash.resetToDefaults()")), 1);
    EXPECT_GT(dashboard.indexOf(QStringLiteral("dash.resetToDefaults()")),
              dashboard.indexOf(QStringLiteral("id: resetConfirm")));
}

TEST(QmlContractTest, ScrubAreaReportsTrackPositionAsFraction) {
    const QString source = loadSource("Views/ScrubArea.qml");
    expectContains(source, QStringLiteral("property real trackWidth"));
    expectContains(source, QStringLiteral("signal scrubStarted(real fraction)"));
    expectContains(source, QStringLiteral("signal scrubMoved(real fraction)"));
    expectContains(source, QStringLiteral("signal scrubEnded(real fraction)"));
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
    expectContains(source, QStringLiteral("modelData.percent / 100"));
}

TEST(QmlContractTest, BundlesTheFontAwesomeFallbackFont) {
    QFile font(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QStringLiteral("/Views/assets/fonts/FontAwesome6-Solid.ttf"));
    EXPECT_TRUE(font.exists()) << "Font Awesome fallback font is missing from the assets";
    EXPECT_GT(font.size(), 0);
}


// --- Pro gating ------------------------------------------------------------

namespace {
void expectNotContains(const QString& source, const QString& forbidden) {
    EXPECT_FALSE(source.contains(forbidden)) << "Behaviour must bind to pro.*, not: " << forbidden.toStdString();
}
} // namespace

TEST(QmlContractTest, GatedBehaviourBindsToTheProGateNotToSettings) {
    const QString widget = loadSource("Views/TaskbarWidget.qml");
    expectContains(widget, QStringLiteral("pro.visualizerEnabled"));
    expectContains(widget, QStringLiteral("pro.taskbarWidgetControlsEnabled"));
    expectContains(widget, QStringLiteral("pro.taskbarWidgetPauseAllEnabled"));
    expectContains(widget, QStringLiteral("pro.taskbarWidgetPlayAllEnabled"));
    expectContains(widget, QStringLiteral("pro.taskbarWidgetShowPauseOverlay"));
    expectContains(widget, QStringLiteral("pro.taskbarWidgetScrollingEnabled"));
    expectNotContains(widget, QStringLiteral("settings.taskbarVisualizerEnabled"));
    expectNotContains(widget, QStringLiteral("settings.taskbarWidgetControlsEnabled"));
    expectNotContains(widget, QStringLiteral("settings.taskbarWidgetPauseAllEnabled"));
    expectNotContains(widget, QStringLiteral("settings.taskbarWidgetPlayAllEnabled"));
    expectNotContains(widget, QStringLiteral("settings.taskbarWidgetShowPauseOverlay"));
    expectNotContains(widget, QStringLiteral("settings.taskbarWidgetScrollingEnabled"));

    expectContains(loadSource("Views/DockableWidget.qml"), QStringLiteral("pro.taskbarWidgetAdaptiveColors"));
    expectContains(loadSource("Views/WidgetStack.qml"), QStringLiteral("pro.taskbarWidgetLayout === 1"));
    expectContains(loadSource("Views/NowPlayingFlyout.qml"), QStringLiteral("pro.nextUpStyle === 1"));

    const QString volume = loadSource("Views/VolumeFlyout.qml");
    expectContains(volume, QStringLiteral("pro.mixerEnabled"));
    expectNotContains(volume, QStringLiteral("settings.volumeFlyoutMixerEnabled"));

    const QString peripherals = loadSource("Views/PeripheralsWidget.qml");
    expectContains(peripherals, QStringLiteral("pollActive: pro.peripheralsWidgetEnabled"));
    expectNotContains(peripherals, QStringLiteral("settings.peripheralsWidgetEnabled"));
    expectContains(loadSource("Views/PeripheralFlyout.qml"), QStringLiteral("!pro.peripheralFlyoutEnabled"));

    const QString app = loadSource("Views/App.qml");
    expectContains(app, QStringLiteral("if(pro.mixerWindowAllowed)"));
    expectContains(app, QStringLiteral("dashboard.showPage(\"home\")"));
}

TEST(QmlContractTest, DashboardTogglesStayOnSettingsAndLockWithTheEntitlement) {
    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("locked: !entitlement.isUnlocked(\"visualizer\")"));
    expectContains(dashboard, QStringLiteral("locked: !entitlement.isUnlocked(\"perAppMixer\")"));
    expectContains(dashboard, QStringLiteral("locked: !entitlement.isUnlocked(\"peripheralBattery\")"));
    expectContains(dashboard, QStringLiteral("locked: !entitlement.isUnlocked(\"nowPlayingStyles\")"));
    expectContains(dashboard, QStringLiteral("locked: !entitlement.isUnlocked(\"taskbarWidgetOptions\")"));
    expectContains(dashboard, QStringLiteral("settings.taskbarVisualizerEnabled = value"));
    expectContains(dashboard, QStringLiteral("settings.volumeFlyoutMixerEnabled = value"));
    expectContains(dashboard, QStringLiteral("settings.nextUpStyle = index"));

    const QRegularExpression knownIds(QStringLiteral("entitlement\\.isUnlocked\\(\"([A-Za-z]+)\"\\)"));
    auto it = knownIds.globalMatch(dashboard);
    const QStringList allowed{QStringLiteral("visualizer"), QStringLiteral("perAppMixer"),
                              QStringLiteral("peripheralBattery"), QStringLiteral("nowPlayingStyles"),
                              QStringLiteral("taskbarWidgetOptions")};
    int found = 0;
    while(it.hasNext()) {
        const QString id = it.next().captured(1);
        EXPECT_TRUE(allowed.contains(id)) << "Unknown feature id in Dashboard.qml: " << id.toStdString();
        ++found;
    }
    EXPECT_GE(found, 5);
}

TEST(QmlContractTest, SettingRowsShowAProBadgeWhenLocked) {
    for(const char* file : {"Views/SettingRow.qml", "Views/SettingCombo.qml"}) {
        const QString source = loadSource(file);
        expectContains(source, QStringLiteral("property bool locked: false"));
        expectContains(source, QStringLiteral("ProBadge {"));
    }
    expectContains(loadSource("Views/ProBadge.qml"), QStringLiteral("qsTr(\"PRO\")"));
}

TEST(QmlContractTest, UpgradeCardIsClickableAndOwnedByTheDashboard) {
    const QString card = loadSource("Views/UpgradeCard.qml");
    expectContains(card, QStringLiteral("visible: entitlement.gatingEnabled && !entitlement.isPro"));
    expectContains(card, QStringLiteral("entitlement.purchase(card.ownerWindow)"));
    expectContains(card, QStringLiteral("enabled: card.clickable"));
    expectContains(card, QStringLiteral("entitlement.trialDaysLeft"));
    expectContains(card, QStringLiteral("entitlement.price"));
    expectContains(card, QStringLiteral("function onPurchaseFinished(ok, statusId, storeId)"));
    expectContains(card, QStringLiteral("storeId === entitlement.proAddOnStoreId"));
    for(const char* status : {"succeeded", "alreadyPurchased", "notPurchased", "networkError", "unavailable",
                              "notConfigured", "busy"})
        expectContains(card, QStringLiteral("case \"%1\"").arg(QLatin1String(status)));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("UpgradeCard {"));
    expectContains(dashboard, QStringLiteral("ownerWindow: dash"));
}

TEST(QmlContractTest, SupportPageFollowsTheInstallKindAndStorePolicy) {
    const QString page = loadSource("Views/SupportPage.qml");
    expectContains(page, QStringLiteral("visible: donate.showDirect"));
    expectContains(page, QStringLiteral("visible: donate.showStoreTips"));
    expectContains(page, QStringLiteral("visible: donate.hasPage"));
    expectContains(page, QStringLiteral("source: donate.upiQrSource"));
    expectContains(page, QStringLiteral("onClicked: donate.openPage()"));
    expectContains(page, QStringLiteral("page.copy(donate.upiId)"));
    expectContains(page, QStringLiteral("model: entitlement.tipAddOnStoreIds"));
    expectContains(page, QStringLiteral("entitlement.purchaseAddOn(modelData, page.ownerWindow)"));
    expectContains(page, QStringLiteral("Microsoft is not the fundraiser or sponsor"));
    expectContains(page, QStringLiteral("Donations are voluntary and unlock nothing."));

    const QString dashboard = loadSource("Views/Dashboard.qml");
    expectContains(dashboard, QStringLiteral("\"key\": \"support\""));
    expectContains(dashboard, QStringLiteral("SupportPage {"));
    expectContains(dashboard, QStringLiteral("visible: dash.currentIndex === 10 && donate.available"));
    expectContains(dashboard, QStringLiteral("visible: dash.currentIndex === 11"));
}
