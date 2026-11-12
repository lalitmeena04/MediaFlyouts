#pragma once

#include <QHash>
#include <QImage>
#include <QLocalServer>
#include <QObject>
#include <QSet>

class QLocalSocket;

class BrowserArtworkBridge : public QObject {
        Q_OBJECT

    public:
        explicit BrowserArtworkBridge(QObject* parent = nullptr);
        ~BrowserArtworkBridge() override;
        bool start(const QString& name = QStringLiteral("MediaFlyouts.browserArtwork"));
        void pauseAllBrowserTabs();

    signals:
        void artworkReceived(const QString& browser, const QString& title, int audibleTabCount, const QImage& image);
        void browserDisconnected(const QString& browser);

    private:
        void acceptConnections();
        void readMessages(QLocalSocket* socket);
        bool handleMessage(QLocalSocket* socket, const QByteArray& message);

        QHash<QString, QSet<QLocalSocket*>> m_clients;
        QLocalServer m_server;
};