#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>

class BrowserIntegration : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY statusChanged)
        Q_PROPERTY(bool developmentAvailable READ developmentAvailable CONSTANT)

    public:
        explicit BrowserIntegration(QObject* parent = nullptr);
        QString status() const;
        bool developmentAvailable() const;
        Q_INVOKABLE bool canInstall(const QString& browser) const;
        Q_INVOKABLE void install(const QString& browser);
        Q_INVOKABLE void openDevelopmentPackage(const QString& browser);

        static QJsonObject nativeManifest(const QString& browser, const QString& hostPath,
                                          const QString& chromiumId);
        static bool writeManifest(const QString& manifestPath, const QJsonObject& manifest);
        static QUrl storeUrl(const QString& browser, const QString& chromiumId, const QString& firefoxUrl);
        static QString externalExtensionRegistryPath(const QString& browser, const QString& chromiumId);
        static QUrl externalExtensionUpdateUrl(const QString& browser);
        static QString nativeHostRegistryPath(const QString& browser);

    signals:
        void statusChanged();

    private:
        bool registerHost(const QString& browser, const QString& chromiumId);
        bool registerExternalExtension(const QString& browser, const QString& chromiumId);
        void setStatus(const QString& status);
        QString m_status;
};