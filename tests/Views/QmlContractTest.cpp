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

    // Fluent covers most peripheral categories
    expectContains(source, QStringLiteral("import MediaFlyouts"));
    expectContains(source, QStringLiteral("Fluent.keyboard"));
    expectContains(source, QStringLiteral("Fluent.gamepad"));
    expectContains(source, QStringLiteral("Fluent.headset"));
    expectContains(source, QStringLiteral("Fluent.headphones"));

    // mouse has no Fluent glyph, so it falls back to the bundled Font Awesome font
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
    // every place that renders a device glyph must also pick the matching font family,
    // otherwise a Font Awesome fallback glyph would render as tofu in the Fluent font
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

TEST(QmlContractTest, FluentBatteryGlyphUsesTheBundledBatteryIcon) {
    const QString source = loadSource("Views/Fluent.qml");
    expectContains(source, QStringLiteral("battery: \"\\uE14E\""));
}

TEST(QmlContractTest, BundlesTheFontAwesomeFallbackFont) {
    QFile font(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QStringLiteral("/Views/assets/fonts/FontAwesome6-Solid.ttf"));
    EXPECT_TRUE(font.exists()) << "Font Awesome fallback font is missing from the assets";
    EXPECT_GT(font.size(), 0);
}

