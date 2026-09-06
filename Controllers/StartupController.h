#pragma once

#include <QObject>
#include <QString>

// Enable or disable auto-launch at logon.
// MSIX builds use StartupTask. Unpackaged builds use Run key.
class StartupController : public QObject {
        Q_OBJECT

    public:
        explicit StartupController(QObject* parent = nullptr);

        Q_INVOKABLE bool isEnabled() const;

    public slots:
        // Make Run entry match enable. Point it at current executable.
        void setEnabled(bool enable);

    private:
        static QString runKeyPath();

        QString m_appName;
};
