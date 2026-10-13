#pragma once

#include <QObject>
#include <QString>

class QLocalServer;

class SingleInstance : public QObject {
        Q_OBJECT

    public:
        explicit SingleInstance(const QString& key, QObject* parent = nullptr);

        bool isPrimary() const;

    signals:
        void activated();

    private:
        QString m_key;
        bool m_primary = false;
        QLocalServer* m_server = nullptr;
};
