#pragma once

#include <QObject>
#include <QString>

// Watch Caps, Num, Scroll, and Insert toggles with low-level keyboard hook.
class LockKeysController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString keyName READ keyName NOTIFY changed)
        Q_PROPERTY(bool on READ on NOTIFY changed)
        // 0 = Caps, 1 = Num, 2 = Scroll, 3 = Insert
        Q_PROPERTY(int keyType READ keyType NOTIFY changed)

    public:
        explicit LockKeysController(QObject* parent = nullptr);
        ~LockKeysController() override;

        void start();

        QString keyName() const;
        bool on() const;
        int keyType() const;
        // True after low-level keyboard hook installs.
        bool available() const;

        // Internal. Called by keyboard hook on main thread.
        void handleToggle(int vk);

    signals:
        void changed();
        void lockKeyToggled();
        void availabilityChanged();

    private:
        QString m_keyName;
        bool m_on = false;
        int m_keyType = 0;
        bool m_available = false;
};
