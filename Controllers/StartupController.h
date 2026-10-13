#pragma once

#include <QObject>
#include <QString>

class StartupController : public QObject {
        Q_OBJECT

    public:
        explicit StartupController(QObject* parent = nullptr);

    public slots:
        void setEnabled(bool enable);

    private:
        static QString runKeyPath();

        QString m_appName;
};
