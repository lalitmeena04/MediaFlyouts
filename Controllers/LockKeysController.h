#pragma once

#include <QObject>
#include <QString>

// Watches Caps/Num/Scroll lock toggles via a low-level keyboard hook and drives the lock-keys flyout.
class LockKeysController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString keyName READ keyName NOTIFY changed)
        Q_PROPERTY(bool on READ on NOTIFY changed)

    public:
        explicit LockKeysController(QObject* parent = nullptr);
        ~LockKeysController() override;

        void start();

        QString keyName() const;
        bool on() const;

        // internal: invoked by the keyboard hook on the main thread
        void handleToggle(int vk);

    signals:
        void changed();
        void lockKeyToggled();

    private:
        QString m_keyName;
        bool m_on = false;
};
