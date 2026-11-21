#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QString>
#include <QVector>

class Settings;

class GlobalShortcutController : public QObject, public QAbstractNativeEventFilter {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY statusChanged)

    public:
        explicit GlobalShortcutController(Settings* settings, QObject* parent = nullptr);
        ~GlobalShortcutController() override;

        QString status() const;
        Q_INVOKABLE void setCaptureMode(bool capturing);
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

    signals:
        void actionTriggered(const QString& action);
        void statusChanged();

    private:
        struct Registration {
            int id = 0;
            QString action;
            bool registered = false;
        };

        void updateBindings();
        void unregisterAll();
        void setStatus(const QString& status);

        Settings* m_settings;
        QVector<Registration> m_registrations;
        QString m_status;
        bool m_captureMode = false;
};
