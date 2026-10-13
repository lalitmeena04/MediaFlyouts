#pragma once

#include <QColor>
#include <QObject>
#include <QTimer>

#include <memory>

class AccentWatcher : public QObject {
        Q_OBJECT

    public:
        explicit AccentWatcher(QObject* parent = nullptr);
        ~AccentWatcher() override;

        QColor accent() const;
        void start();

    signals:
        void accentChanged(QColor accent);

    private:
        void refresh();

        struct Impl;
        std::unique_ptr<Impl> m_impl;
        QTimer m_timer;
        QColor m_accent;
};
