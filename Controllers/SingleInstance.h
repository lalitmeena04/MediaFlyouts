#pragma once

#include <QObject>
#include <QString>

class QLocalServer;

// Ensures only one MediaFlyouts runs; a second launch signals the first to show its dashboard.
class SingleInstance : public QObject {
        Q_OBJECT

    public:
        explicit SingleInstance(const QString& key, QObject* parent = nullptr);

        bool isPrimary() const;

    signals:
        // another instance tried to start and asked us to surface the dashboard
        void activated();

    private:
        QString m_key;
        bool m_primary = false;
        QLocalServer* m_server = nullptr;
};
