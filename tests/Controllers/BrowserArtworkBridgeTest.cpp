#include <gtest/gtest.h>

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QLocalServer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSignalSpy>
#include <QTest>
#include <QUuid>
#include <QtEndian>

#include "BrowserArtworkBridge.h"

TEST(BrowserArtworkBridgeTest, ReceivesArtworkAndClearsOnDisconnect) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QSignalSpy received(&bridge, &BrowserArtworkBridge::artworkReceived);
    QSignalSpy disconnected(&bridge, &BrowserArtworkBridge::browserDisconnected);
    QLocalSocket client;
    client.connectToServer(name);
    ASSERT_TRUE(client.waitForConnected(1000));
    QImage image(4, 4, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QByteArray imageBytes;
    QBuffer buffer(&imageBytes);
    ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
    ASSERT_TRUE(image.save(&buffer, "PNG"));
    const QJsonObject message{{"browser", "brave"}, {"title", "Song"}, {"audibleCount", 1},
                              {"icon", QString::fromLatin1(imageBytes.toBase64())}};
    client.write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
    client.flush();
    ASSERT_TRUE(received.wait(1000));
    EXPECT_EQ(received.at(0).at(0).toString(), QStringLiteral("brave"));
    EXPECT_EQ(received.at(0).at(3).value<QImage>().pixelColor(0, 0), QColor(Qt::blue));
    client.disconnectFromServer();
    ASSERT_TRUE(disconnected.wait(1000));
    EXPECT_EQ(disconnected.at(0).at(0).toString(), QStringLiteral("brave"));
}

TEST(BrowserArtworkBridgeTest, SendsPauseCommandToConnectedExtension) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QSignalSpy received(&bridge, &BrowserArtworkBridge::artworkReceived);
    QLocalSocket client;
    client.connectToServer(name);
    ASSERT_TRUE(client.waitForConnected(1000));
    client.write(R"({"browser":"chrome","title":"Song","audibleCount":1})" "\n");
    client.flush();
    ASSERT_TRUE(received.wait(1000));

    bridge.pauseAllBrowserTabs();
    ASSERT_TRUE(client.waitForReadyRead(1000));
    const QJsonObject command =
        QJsonDocument::fromJson(client.readLine().trimmed()).object();
    EXPECT_EQ(command.value(QStringLiteral("command")).toString(), QStringLiteral("pauseAudibleTabs"));
}

TEST(BrowserArtworkBridgeTest, RejectsInvalidBrowser) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QSignalSpy received(&bridge, &BrowserArtworkBridge::artworkReceived);
    QLocalSocket client;
    client.connectToServer(name);
    ASSERT_TRUE(client.waitForConnected(1000));
    client.write("{\"browser\":\"unknown\",\"audibleCount\":1}\n");
    client.flush();
    QSignalSpy disconnected(&client, &QLocalSocket::disconnected);
    ASSERT_TRUE(disconnected.wait(1000));
    EXPECT_EQ(received.size(), 0);
}

TEST(BrowserArtworkBridgeTest, MultipleBrowserProfilesClearTheCandidate) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QSignalSpy received(&bridge, &BrowserArtworkBridge::artworkReceived);
    QSignalSpy cleared(&bridge, &BrowserArtworkBridge::browserDisconnected);
    QLocalSocket first;
    QLocalSocket second;
    first.connectToServer(name);
    second.connectToServer(name);
    ASSERT_TRUE(first.waitForConnected(1000));
    ASSERT_TRUE(second.waitForConnected(1000));
    const QByteArray message = "{\"browser\":\"chrome\",\"title\":\"Song\",\"audibleCount\":1}\n";
    first.write(message);
    first.flush();
    ASSERT_TRUE(received.wait(1000));
    second.write(message);
    second.flush();
    ASSERT_TRUE(cleared.wait(1000));
    EXPECT_EQ(received.size(), 1);
    EXPECT_EQ(cleared.at(0).at(0).toString(), QStringLiteral("chrome"));
}

TEST(BrowserArtworkBridgeTest, RejectsOversizedMessages) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QLocalSocket client;
    client.connectToServer(name);
    ASSERT_TRUE(client.waitForConnected(1000));
    QSignalSpy disconnected(&client, &QLocalSocket::disconnected);
    client.write(QByteArray(128 * 1024 + 1, 'x'));
    client.flush();
    ASSERT_TRUE(disconnected.wait(1000));
}

#ifdef Q_OS_WIN
TEST(BrowserArtworkBridgeTest, NativeHostExitsWhenAppIsUnavailable) {
    QProcess host;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("MEDIAFLYOUTS_BROWSER_ART_SERVER"),
                       QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128));
    host.setProcessEnvironment(environment);
    host.start(QStringLiteral(MEDIAFLYOUTS_BROWSER_HOST_PATH));
    ASSERT_TRUE(host.waitForStarted(2000));
    ASSERT_TRUE(host.waitForFinished(3000));
    EXPECT_EQ(host.exitStatus(), QProcess::NormalExit);
    EXPECT_EQ(host.exitCode(), 0);
    EXPECT_TRUE(host.readAllStandardError().contains("exiting"));
}

TEST(BrowserArtworkBridgeTest, NativeHostExitsWhenAppDisconnectsWithBrowserInputOpen) {
    for(const QByteArray& input : {QByteArray(), QByteArray("\x20\x00", 2),
                                  QByteArray("\x20\x00\x00\x00{", 5)}) {
        QLocalServer server;
        const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
        ASSERT_TRUE(server.listen(name));
        QProcess host;
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("MEDIAFLYOUTS_BROWSER_ART_SERVER"), name);
        host.setProcessEnvironment(environment);
        host.start(QStringLiteral(MEDIAFLYOUTS_BROWSER_HOST_PATH));
        ASSERT_TRUE(host.waitForStarted(2000));
        ASSERT_TRUE(server.waitForNewConnection(3000));
        QLocalSocket* client = server.nextPendingConnection();
        ASSERT_NE(client, nullptr);
        if(!input.isEmpty()) {
            ASSERT_EQ(host.write(input), input.size());
            ASSERT_TRUE(host.waitForBytesWritten(1000));
            QTest::qWait(100);
        }
        client->abort();
        ASSERT_TRUE(host.waitForFinished(3000));
        EXPECT_EQ(host.exitStatus(), QProcess::NormalExit);
        EXPECT_EQ(host.exitCode(), 0);
    }
}

TEST(BrowserArtworkBridgeTest, NativeHostExitsWhenBrowserDisconnectsWithoutSendingMessages) {
    QLocalServer server;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(server.listen(name));
    QProcess host;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("MEDIAFLYOUTS_BROWSER_ART_SERVER"), name);
    host.setProcessEnvironment(environment);
    host.start(QStringLiteral(MEDIAFLYOUTS_BROWSER_HOST_PATH));
    ASSERT_TRUE(host.waitForStarted(2000));
    ASSERT_TRUE(server.waitForNewConnection(3000));
    host.closeWriteChannel();
    ASSERT_TRUE(host.waitForFinished(3000));
    EXPECT_EQ(host.exitStatus(), QProcess::NormalExit);
    EXPECT_EQ(host.exitCode(), 0);
}

TEST(BrowserArtworkBridgeTest, NativeHostForwardsBrowserMessages) {
    BrowserArtworkBridge bridge;
    const QString name = QStringLiteral("MediaFlyouts.test.") + QUuid::createUuid().toString(QUuid::Id128);
    ASSERT_TRUE(bridge.start(name));
    QSignalSpy received(&bridge, &BrowserArtworkBridge::artworkReceived);
    QSignalSpy disconnected(&bridge, &BrowserArtworkBridge::browserDisconnected);
    QProcess host;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("MEDIAFLYOUTS_BROWSER_ART_SERVER"), name);
    host.setProcessEnvironment(environment);
    host.start(QStringLiteral(MEDIAFLYOUTS_BROWSER_HOST_PATH));
    ASSERT_TRUE(host.waitForStarted(2000));
    const QByteArray message = R"({"browser":"firefox","title":"Song","audibleCount":0,"icon":""})";
    const quint32 size = qToLittleEndian(static_cast<quint32>(message.size()));
    host.write(reinterpret_cast<const char*>(&size), sizeof(size));
    host.write(message);
    ASSERT_TRUE(received.wait(3000));
    EXPECT_EQ(received.at(0).at(0).toString(), QStringLiteral("firefox"));
    bridge.pauseAllBrowserTabs();
    ASSERT_TRUE(host.waitForReadyRead(3000));
    QByteArray response = host.readAllStandardOutput();
    QTRY_VERIFY_WITH_TIMEOUT(response.size() >= static_cast<qsizetype>(sizeof(quint32)), 1000);
    const quint32 responseSize =
        qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(response.constData()));
    QTRY_VERIFY_WITH_TIMEOUT(response.size() >= static_cast<qsizetype>(sizeof(quint32) + responseSize), 1000);
    const QJsonObject command =
        QJsonDocument::fromJson(response.mid(sizeof(quint32), static_cast<qsizetype>(responseSize))).object();
    EXPECT_EQ(command.value(QStringLiteral("command")).toString(), QStringLiteral("pauseAudibleTabs"));
    host.closeWriteChannel();
    ASSERT_TRUE(disconnected.wait(3000));
    ASSERT_TRUE(host.waitForFinished(3000));
    EXPECT_EQ(host.exitCode(), 0);
}
#endif