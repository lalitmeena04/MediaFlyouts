#pragma once

#include <QColor>
#include <QObject>

class Settings;

// Exposes a semantic color palette to QML that follows the appTheme setting
// (0 = system, 1 = light, 2 = dark). QML singletons can't read context
// properties, so this lives as the context object "theme".
class ThemeController : public QObject {
        Q_OBJECT

        Q_PROPERTY(bool dark READ dark NOTIFY changed)
        Q_PROPERTY(QColor window READ window NOTIFY changed)
        Q_PROPERTY(QColor sidebar READ sidebar NOTIFY changed)
        Q_PROPERTY(QColor card READ card NOTIFY changed)
        Q_PROPERTY(QColor cardHover READ cardHover NOTIFY changed)
        Q_PROPERTY(QColor border READ border NOTIFY changed)
        Q_PROPERTY(QColor flyout READ flyout NOTIFY changed)
        Q_PROPERTY(QColor flyoutBorder READ flyoutBorder NOTIFY changed)
        Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY changed)
        Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY changed)
        Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY changed)
        Q_PROPERTY(QColor textMuted READ textMuted NOTIFY changed)
        Q_PROPERTY(QColor accent READ accent NOTIFY changed)
        Q_PROPERTY(QColor accentSoft READ accentSoft NOTIFY changed)
        Q_PROPERTY(QColor accentBadge READ accentBadge NOTIFY changed)
        Q_PROPERTY(QColor artPlaceholder READ artPlaceholder NOTIFY changed)
        Q_PROPERTY(QColor track READ track NOTIFY changed)

    public:
        explicit ThemeController(Settings* settings, QObject* parent = nullptr);

        bool dark() const;
        QColor window() const;
        QColor sidebar() const;
        QColor card() const;
        QColor cardHover() const;
        QColor border() const;
        QColor flyout() const;
        QColor flyoutBorder() const;
        QColor textPrimary() const;
        QColor textSecondary() const;
        QColor textTertiary() const;
        QColor textMuted() const;
        QColor accent() const;
        QColor accentSoft() const;
        QColor accentBadge() const;
        QColor artPlaceholder() const;
        QColor track() const;

    public slots:
        // recompute from the setting / system scheme and notify QML if it changed
        void refresh();
        // the OS accent color, fed by a Windows accent watcher
        void setSystemAccent(const QColor& color);

    signals:
        void changed();

    private:
        bool computeDark() const;

        Settings* m_settings = nullptr;
        bool m_dark = true;
        QColor m_systemAccent;
};
