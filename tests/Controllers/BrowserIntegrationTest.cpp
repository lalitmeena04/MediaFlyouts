#include <gtest/gtest.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>

#include "BrowserIntegration.h"

TEST(BrowserIntegrationTest, NativeManifestsRestrictAuthorizedExtensions) {
    const QString id = QStringLiteral("pmblljpkoajpojjjeidepiddahgpilge");
    const QJsonObject chrome = BrowserIntegration::nativeManifest(QStringLiteral("chrome"), QStringLiteral("C:/host.exe"), id);
    ASSERT_FALSE(chrome.isEmpty());
    EXPECT_FALSE(BrowserIntegration::nativeManifest(QStringLiteral("brave"), QStringLiteral("C:/host.exe"), id).isEmpty());
    EXPECT_EQ(chrome.value(QStringLiteral("allowed_origins")).toArray().at(0).toString(),
              QStringLiteral("chrome-extension://pmblljpkoajpojjjeidepiddahgpilge/"));
    const QJsonObject firefox = BrowserIntegration::nativeManifest(QStringLiteral("firefox"), QStringLiteral("C:/host.exe"), {});
    EXPECT_EQ(firefox.value(QStringLiteral("allowed_extensions")).toArray().at(0).toString(),
              QStringLiteral("browser-artwork@mediaflyouts.local"));
    EXPECT_TRUE(BrowserIntegration::nativeManifest(QStringLiteral("chrome"), QStringLiteral("C:/host.exe"), QStringLiteral("bad")).isEmpty());
    EXPECT_TRUE(BrowserIntegration::nativeManifest(QStringLiteral("other"), QStringLiteral("C:/host.exe"), id).isEmpty());
}

TEST(BrowserIntegrationTest, WritesValidManifestAtomically) {
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("nested/host.json"));
    const QJsonObject manifest = BrowserIntegration::nativeManifest(QStringLiteral("firefox"), QStringLiteral("C:/host.exe"), {});
    ASSERT_TRUE(BrowserIntegration::writeManifest(path, manifest));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QJsonDocument::fromJson(file.readAll()).object(), manifest);
    EXPECT_FALSE(BrowserIntegration::writeManifest(path, {}));
}

TEST(BrowserIntegrationTest, UnknownBrowsersCannotInstallExtensions) {
    BrowserIntegration integration;
    EXPECT_FALSE(integration.canInstall(QStringLiteral("unknown")));
    integration.install(QStringLiteral("unknown"));
    EXPECT_FALSE(integration.status().isEmpty());
}

TEST(BrowserIntegrationTest, StoreUrlsAreRestrictedToTheSelectedBrowser) {
    const QString id = QStringLiteral("pmblljpkoajpojjjeidepiddahgpilge");
    EXPECT_EQ(BrowserIntegration::storeUrl(QStringLiteral("chrome"), id, {}).host(),
              QStringLiteral("chromewebstore.google.com"));
    EXPECT_EQ(BrowserIntegration::storeUrl(QStringLiteral("brave"), id, {}).host(),
              QStringLiteral("chromewebstore.google.com"));
    EXPECT_EQ(BrowserIntegration::storeUrl(QStringLiteral("msedge"), id, {}).host(),
              QStringLiteral("microsoftedge.microsoft.com"));
    EXPECT_EQ(BrowserIntegration::storeUrl(QStringLiteral("firefox"), {},
                                            QStringLiteral("https://addons.mozilla.org/firefox/addon/example/")).host(),
              QStringLiteral("addons.mozilla.org"));
    EXPECT_TRUE(BrowserIntegration::storeUrl(QStringLiteral("firefox"), {}, QStringLiteral("https://example.com/")).isEmpty());
    EXPECT_TRUE(BrowserIntegration::storeUrl(QStringLiteral("chrome"), QStringLiteral("invalid"), {}).isEmpty());
    EXPECT_TRUE(BrowserIntegration::storeUrl(QStringLiteral("other"), id, {}).isEmpty());
}

TEST(BrowserIntegrationTest, ExternalRegistrationUsesBrowserSpecificRegistryAndUpdateUrls) {
    const QString id = QStringLiteral("pmblljpkoajpojjjeidepiddahgpilge");
    EXPECT_EQ(BrowserIntegration::externalExtensionRegistryPath(QStringLiteral("chrome"), id),
              QStringLiteral("Software\\Google\\Chrome\\Extensions\\pmblljpkoajpojjjeidepiddahgpilge"));
    EXPECT_EQ(BrowserIntegration::externalExtensionRegistryPath(QStringLiteral("msedge"), id),
              QStringLiteral("Software\\Microsoft\\Edge\\Extensions\\pmblljpkoajpojjjeidepiddahgpilge"));
    EXPECT_EQ(BrowserIntegration::externalExtensionRegistryPath(QStringLiteral("brave"), id),
              QStringLiteral("Software\\BraveSoftware\\Brave\\Extensions\\pmblljpkoajpojjjeidepiddahgpilge"));
    EXPECT_TRUE(BrowserIntegration::externalExtensionRegistryPath(QStringLiteral("firefox"), id).isEmpty());
    EXPECT_TRUE(BrowserIntegration::externalExtensionRegistryPath(QStringLiteral("chrome"), QStringLiteral("bad")).isEmpty());
    EXPECT_EQ(BrowserIntegration::externalExtensionUpdateUrl(QStringLiteral("chrome")).toString(),
              QStringLiteral("https://clients2.google.com/service/update2/crx"));
    EXPECT_EQ(BrowserIntegration::externalExtensionUpdateUrl(QStringLiteral("brave")).toString(),
              QStringLiteral("https://clients2.google.com/service/update2/crx"));
    EXPECT_EQ(BrowserIntegration::externalExtensionUpdateUrl(QStringLiteral("msedge")).toString(),
              QStringLiteral("https://edge.microsoft.com/extensionwebstorebase/v1/crx"));
    EXPECT_TRUE(BrowserIntegration::externalExtensionUpdateUrl(QStringLiteral("firefox")).isEmpty());
    EXPECT_EQ(
        BrowserIntegration::nativeHostRegistryPath(QStringLiteral("brave")),
        QStringLiteral("Software\\BraveSoftware\\Brave-Browser\\NativeMessagingHosts\\com.mediaflyouts.browser_artwork"));
    EXPECT_TRUE(BrowserIntegration::nativeHostRegistryPath(QStringLiteral("unknown")).isEmpty());
}