#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <thread>

#include <QCoreApplication>
#include <QLocalSocket>
#include <QMetaObject>
#include <QQueue>
#include <QTimer>
#include <QtEndian>

namespace {
constexpr quint32 kMaxMessageBytes = 128 * 1024;
constexpr int kMaxQueuedBytes = 4 * 1024 * 1024;

bool readMessage(QByteArray& message) {
    quint32 wireLength = 0;
    if(std::fread(&wireLength, 1, sizeof(wireLength), stdin) != sizeof(wireLength))
        return false;
    const quint32 length = qFromLittleEndian(wireLength);
    if(length == 0 || length > kMaxMessageBytes)
        return false;
    message.resize(static_cast<qsizetype>(length));
    return std::fread(message.data(), 1, length, stdin) == length && !message.contains('\n');
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);

    const QString serverName = qEnvironmentVariable("MEDIAFLYOUTS_BROWSER_ART_SERVER",
                                                     QStringLiteral("MediaFlyouts.browserArtwork"));
    QLocalSocket socket;
    QQueue<QByteArray> pending;
    int pendingBytes = 0;

    const auto flushPending = [&] {
        while(socket.state() == QLocalSocket::ConnectedState && !pending.isEmpty()) {
            const QByteArray message = pending.head() + '\n';
            if(socket.write(message) < 0)
                break;
            pendingBytes -= pending.head().size();
            pending.dequeue();
        }
    };

    QObject::connect(&socket, &QLocalSocket::connected, &app, flushPending);

    QTimer reconnect;
    reconnect.setInterval(500);
    QObject::connect(&reconnect, &QTimer::timeout, &app, [&] {
        if(socket.state() == QLocalSocket::UnconnectedState)
            socket.connectToServer(serverName);
    });
    reconnect.start();
    socket.connectToServer(serverName);

    std::thread inputThread([&] {
        QByteArray message;
        while(readMessage(message)) {
            QMetaObject::invokeMethod(
                &app,
                [&socket, &pending, &pendingBytes, message = std::move(message)] {
                    if(pendingBytes + message.size() > kMaxQueuedBytes) {
                        std::fprintf(stderr, "MediaFlyouts browser host dropped an oversized queue\n");
                        return;
                    }
                    pendingBytes += message.size();
                    pending.enqueue(message);
                    while(socket.state() == QLocalSocket::ConnectedState && !pending.isEmpty()) {
                        const QByteArray framed = pending.head() + '\n';
                        if(socket.write(framed) < 0)
                            break;
                        pendingBytes -= pending.head().size();
                        pending.dequeue();
                    }
                },
                Qt::QueuedConnection);
        }
        QMetaObject::invokeMethod(&app, [&app] { app.quit(); }, Qt::QueuedConnection);
    });

    const int result = app.exec();
    inputThread.join();
    return result;
}
