#pragma once

#include <QObject>
#include <QString>

class QLocalServer;

// Ensure only one MediaFlyouts instance runs.
// Second launch signals the first instance.
class SingleInstance : public QObject {
        Q_OBJECT

    public:
        explicit SingleInstance(const QString& key, QObject* parent = nullptr);

        bool isPrimary() const;

    signals:
        // Another instance asked this instance to show dashboard.
        void activated();

    private:
        QString m_key;
        bool m_primary = false;
        QLocalServer* m_server = nullptr;
};
