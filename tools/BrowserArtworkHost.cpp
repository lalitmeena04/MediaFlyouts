#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QTimer>
#include <QtEndian>

namespace {
constexpr quint32 kMaxMessageBytes = 128 * 1024;
constexpr int kMaxQueuedBytes = 4 * 1024 * 1024;

bool writeMessage(const QByteArray& message) {
    if(message.isEmpty() || message.size() > static_cast<qsizetype>(kMaxMessageBytes))
        return false;
    const quint32 wireLength = qToLittleEndian(static_cast<quint32>(message.size()));
    return std::fwrite(&wireLength, 1, sizeof(wireLength), stdout) == sizeof(wireLength) &&
           std::fwrite(message.constData(), 1, static_cast<size_t>(message.size()), stdout) ==
               static_cast<size_t>(message.size()) &&
           std::fflush(stdout) == 0;
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);

    const QString serverName = qEnvironmentVariable("MEDIAFLYOUTS_BROWSER_ART_SERVER",
                                                     QStringLiteral("MediaFlyouts.browserArtwork"));
    QLocalSocket socket;
    socket.setReadBufferSize(kMaxMessageBytes + 1);
    QByteArray inputBuffer;
    QByteArray responseBuffer;

    QObject::connect(&socket, &QLocalSocket::disconnected, &app, [&] {
        std::fprintf(stderr, "MediaFlyouts browser host exiting: app connection closed\n");
        app.quit();
    });
    QObject::connect(&socket, &QLocalSocket::errorOccurred, &app, [&](QLocalSocket::LocalSocketError) {
        std::fprintf(stderr, "MediaFlyouts browser host exiting: %s\n",
                     qPrintable(socket.errorString()));
        app.quit();
    });
    QObject::connect(&socket, &QLocalSocket::readyRead, &app, [&] {
        responseBuffer.append(socket.readAll());
        while(true) {
            const qsizetype end = responseBuffer.indexOf('\n');
            if(end < 0)
                break;
            if(end == 0 || end > kMaxMessageBytes ||
               !QJsonDocument::fromJson(responseBuffer.first(end)).isObject()) {
                std::fprintf(stderr, "MediaFlyouts browser host received an invalid app message\n");
                app.exit(1);
                return;
            }
            if(!writeMessage(responseBuffer.first(end))) {
                std::fprintf(stderr, "MediaFlyouts browser host could not write a native message\n");
                app.exit(1);
                return;
            }
            responseBuffer.remove(0, end + 1);
        }
        if(responseBuffer.size() > static_cast<qsizetype>(kMaxMessageBytes)) {
            std::fprintf(stderr, "MediaFlyouts browser host received an oversized app message\n");
            app.exit(1);
        }
    });

    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    QTimer inputPoll;
    inputPoll.setInterval(25);
    QObject::connect(&inputPoll, &QTimer::timeout, &app, [&] {
        DWORD available = 0;
        if(!PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr)) {
            const DWORD error = GetLastError();
            if(error == ERROR_BROKEN_PIPE) {
                std::fprintf(stderr, "MediaFlyouts browser host exiting: browser connection closed\n");
                app.quit();
            } else {
                std::fprintf(stderr, "MediaFlyouts browser host could not check browser input: %lu\n", error);
                app.exit(1);
            }
            return;
        }
        if(available == 0 || socket.state() != QLocalSocket::ConnectedState)
            return;
        // Read only buffered pipe bytes so app shutdown never waits for browser input.
        char bytes[16 * 1024];
        DWORD count = 0;
        const DWORD length = qMin(available, static_cast<DWORD>(sizeof(bytes)));
        if(!ReadFile(input, bytes, length, &count, nullptr)) {
            std::fprintf(stderr, "MediaFlyouts browser host could not read browser input: %lu\n", GetLastError());
            app.exit(1);
            return;
        }
        inputBuffer.append(bytes, static_cast<qsizetype>(count));
        while(inputBuffer.size() >= static_cast<qsizetype>(sizeof(quint32))) {
            const quint32 messageSize =
                qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(inputBuffer.constData()));
            if(messageSize == 0 || messageSize > kMaxMessageBytes) {
                std::fprintf(stderr, "MediaFlyouts browser host received an invalid browser message size\n");
                app.exit(1);
                return;
            }
            const qsizetype frameSize = sizeof(quint32) + messageSize;
            if(inputBuffer.size() < frameSize)
                break;
            const QByteArray message = inputBuffer.mid(sizeof(quint32), messageSize);
            if(message.contains('\n') || socket.bytesToWrite() + message.size() + 1 > kMaxQueuedBytes) {
                std::fprintf(stderr, "MediaFlyouts browser host rejected an invalid or oversized browser message\n");
                app.exit(1);
                return;
            }
            if(socket.write(message + '\n') < 0) {
                std::fprintf(stderr, "MediaFlyouts browser host could not forward a browser message\n");
                app.exit(1);
                return;
            }
            inputBuffer.remove(0, frameSize);
        }
    });
    inputPoll.start();
    QTimer::singleShot(0, &socket, [&] { socket.connectToServer(serverName); });

    return app.exec();
}
