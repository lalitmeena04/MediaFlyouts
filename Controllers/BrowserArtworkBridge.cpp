#include "BrowserArtworkBridge.h"

#include <QBuffer>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QTimer>

#include "Logging.h"

namespace {
constexpr int kMaxMessageBytes = 128 * 1024;
constexpr int kMaxImageDimension = 512;
constexpr int kClientTimeoutMs = 90000;
}

BrowserArtworkBridge::BrowserArtworkBridge(QObject* parent) : QObject(parent) {
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&m_server, &QLocalServer::newConnection, this, &BrowserArtworkBridge::acceptConnections);
}

bool BrowserArtworkBridge::start(const QString& name) {
    if(!m_server.listen(name)) {
        TRC_WARN("browser artwork: cannot start local server: {}", m_server.errorString().toStdString());
        return false;
    }
    return true;
}

void BrowserArtworkBridge::pauseAllBrowserTabs() {
    const QByteArray command = R"({"command":"pauseAudibleTabs"})" "\n";
    int sent = 0;
    for(auto it = m_clients.cbegin(); it != m_clients.cend(); ++it) {
        for(QLocalSocket* socket : it.value()) {
            if(!socket || socket->state() != QLocalSocket::ConnectedState)
                continue;
            if(socket->write(command) < 0) {
                TRC_WARN("browser artwork: could not send pause command to {}", it.key().toStdString());
                continue;
            }
            ++sent;
        }
    }
    TRC_DEBUG("browser artwork: pause command sent to {} extension(s)", sent);
}

void BrowserArtworkBridge::acceptConnections() {
    while(m_server.hasPendingConnections()) {
        auto* socket = m_server.nextPendingConnection();
        socket->setReadBufferSize(kMaxMessageBytes + 1);
        auto* timer = new QTimer(socket);
        timer->setSingleShot(true);
        timer->start(kClientTimeoutMs);
        connect(timer, &QTimer::timeout, socket, &QLocalSocket::disconnectFromServer);
        connect(socket, &QLocalSocket::readyRead, this, [this, socket, timer] {
            timer->start(kClientTimeoutMs);
            readMessages(socket);
        });
        connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
            const QString browser = socket->property("browser").toString();
            if(m_clients.contains(browser)) {
                m_clients[browser].remove(socket);
                if(m_clients[browser].isEmpty())
                    m_clients.remove(browser);
                emit browserDisconnected(browser);
            }
            socket->deleteLater();
        });
    }
}

void BrowserArtworkBridge::readMessages(QLocalSocket* socket) {
    QByteArray pending = socket->property("pending").toByteArray();
    pending.append(socket->readAll());
    while(true) {
        const qsizetype end = pending.indexOf('\n');
        if(end < 0)
            break;
        if(end > kMaxMessageBytes || !handleMessage(socket, pending.left(end))) {
            TRC_WARN("browser artwork: rejected invalid client message");
            socket->disconnectFromServer();
            return;
        }
        pending.remove(0, end + 1);
    }
    if(pending.size() > kMaxMessageBytes) {
        TRC_WARN("browser artwork: client message exceeds limit");
        socket->disconnectFromServer();
        return;
    }
    socket->setProperty("pending", pending);
}

bool BrowserArtworkBridge::handleMessage(QLocalSocket* socket, const QByteArray& message) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(message, &error);
    if(error.error != QJsonParseError::NoError || !document.isObject())
        return false;
    const QJsonObject object = document.object();
    const QString browser = object.value(QStringLiteral("browser")).toString();
    if(browser != QStringLiteral("chrome") && browser != QStringLiteral("msedge") &&
         browser != QStringLiteral("firefox") && browser != QStringLiteral("brave"))
        return false;
    const QString previous = socket->property("browser").toString();
    if(!previous.isEmpty() && previous != browser)
        return false;
    const QString title = object.value(QStringLiteral("title")).toString();
    const int count = object.value(QStringLiteral("audibleCount")).toInt(-1);
    if(title.size() > 2048 || count < 0 || count > 1000)
        return false;

    QImage image;
    const QString encoded = object.value(QStringLiteral("icon")).toString();
    if(!encoded.isEmpty()) {
        const auto bytes = QByteArray::fromBase64Encoding(encoded.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if(!bytes)
            return false;
        QBuffer buffer;
        buffer.setData(bytes.decoded);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        const QSize size = reader.size();
        if(!size.isValid() || size.width() > kMaxImageDimension || size.height() > kMaxImageDimension)
            return false;
        image = reader.read();
        if(image.isNull())
            return false;
    }
    socket->setProperty("browser", browser);
    m_clients[browser].insert(socket);
    TRC_DEBUG("browser artwork: browser={} audible={} image={} clients={}", browser.toStdString(), count,
              !image.isNull(), m_clients.value(browser).size());
    if(m_clients.value(browser).size() == 1)
        emit artworkReceived(browser, title, count, image);
    else
        emit browserDisconnected(browser);
    return true;
}

BrowserArtworkBridge::~BrowserArtworkBridge() {
    for(const auto& clients : m_clients) {
        for(QLocalSocket* socket : clients)
            disconnect(socket, nullptr, this, nullptr);
    }
    m_server.close();
}