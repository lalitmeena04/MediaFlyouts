#pragma once

#include <QObject>
#include <QString>

// Enables/disables auto-launch at logon. Packaged (MSIX) builds use the declared StartupTask
// (the Run registry key is not honored for packaged apps); unpackaged builds use the Run key.
class StartupController : public QObject {
        Q_OBJECT

    public:
        explicit StartupController(QObject* parent = nullptr);

        Q_INVOKABLE bool isEnabled() const;

    public slots:
        // ensure the Run entry matches 'enable' and points at the current executable
        void setEnabled(bool enable);

    private:
        static QString runKeyPath();

        QString m_appName;
};
