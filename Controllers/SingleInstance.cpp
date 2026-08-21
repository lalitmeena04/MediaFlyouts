#include "SingleInstance.h"

#include <QLocalServer>
#include <QLocalSocket>

SingleInstance::SingleInstance(const QString& key, QObject* parent) : QObject(parent), m_key(key) {
    // if we can connect to an existing server, another instance already owns the key
    QLocalSocket socket;
    socket.connectToServer(m_key);
    if(socket.waitForConnected(200)) {
        socket.write("show");
        socket.waitForBytesWritten(200);
        socket.disconnectFromServer();
        m_primary = false;
        return;
    }

    // a stale socket can linger after a crash; clear it before listening
    QLocalServer::removeServer(m_key);
    m_server = new QLocalServer(this);
    m_primary = true; // run even if listen() fails, rather than block the user
    if(m_server->listen(m_key)) {
        connect(m_server, &QLocalServer::newConnection, this, [this] {
            if(QLocalSocket* c = m_server->nextPendingConnection()) {
                c->close();
                c->deleteLater();
            }
            emit activated();
        });
    }
}

bool SingleInstance::isPrimary() const {
    return m_primary;
}
