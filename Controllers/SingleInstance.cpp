#include "SingleInstance.h"

#include <QLocalServer>
#include <QLocalSocket>

#include "Logging.h"

SingleInstance::SingleInstance(const QString& key, QObject* parent) : QObject(parent), m_key(key) {
    // Existing server means another instance owns key.
    QLocalSocket socket;
    socket.connectToServer(m_key);
    if(socket.waitForConnected(200)) {
        TRC_INFO("single instance: another instance is running, handing over");
        socket.write("show");
        socket.waitForBytesWritten(200);
        socket.disconnectFromServer();
        m_primary = false;
        return;
    }

    // Stale socket can remain after crash. Clear it before listen.
    QLocalServer::removeServer(m_key);
    m_server = new QLocalServer(this);
    m_primary = true; // Run even if listen() fails.
    if(m_server->listen(m_key)) {
        connect(m_server, &QLocalServer::newConnection, this, [this] {
            if(QLocalSocket* c = m_server->nextPendingConnection()) {
                c->close();
                c->deleteLater();
            }
            TRC_INFO("single instance: second launch signalled");
            emit activated();
        });
    } else {
        TRC_WARN("single instance: listen failed: {}", m_server->errorString().toStdString());
    }
}

bool SingleInstance::isPrimary() const {
    return m_primary;
}
