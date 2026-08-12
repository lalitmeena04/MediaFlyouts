#pragma once

#include <QObject>
#include <QString>

// Adds or removes MediaFlyouts from the per-user Windows startup (Run) registry key.
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
