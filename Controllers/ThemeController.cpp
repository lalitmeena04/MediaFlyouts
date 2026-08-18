#include "ThemeController.h"

#include <QGuiApplication>
#include <QStyleHints>

#include "Settings.h"

ThemeController::ThemeController(Settings* settings, QObject* parent) : QObject(parent), m_settings(settings) {
    m_dark = computeDark();
    if(m_settings)
        connect(m_settings, &Settings::changed, this, &ThemeController::refresh);
    if(const QStyleHints* hints = QGuiApplication::styleHints())
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) { refresh(); });
}

bool ThemeController::computeDark() const {
    const int mode = m_settings ? m_settings->appTheme() : 0;
    if(mode == 1)
        return false;
    if(mode == 2)
        return true;
    // system: follow the OS color scheme
    if(const QStyleHints* hints = QGuiApplication::styleHints())
        return hints->colorScheme() != Qt::ColorScheme::Light;
    return true;
}

void ThemeController::refresh() {
    const bool dark = computeDark();
    if(dark == m_dark)
        return;
    m_dark = dark;
    emit changed();
}

bool ThemeController::dark() const {
    return m_dark;
}

QColor ThemeController::window() const {
    return m_dark ? QColor("#1B1B1B") : QColor("#F3F3F3");
}
QColor ThemeController::sidebar() const {
    return m_dark ? QColor("#151515") : QColor("#E8E8E8");
}
QColor ThemeController::card() const {
    return m_dark ? QColor("#232323") : QColor("#FFFFFF");
}
QColor ThemeController::cardHover() const {
    return m_dark ? QColor("#2A2A2A") : QColor("#ECECEC");
}
QColor ThemeController::border() const {
    return m_dark ? QColor("#2E2E2E") : QColor("#D8D8D8");
}
QColor ThemeController::flyout() const {
    return m_dark ? QColor("#EE1E1E1E") : QColor("#EEF3F3F3");
}
QColor ThemeController::flyoutBorder() const {
    return m_dark ? QColor("#22FFFFFF") : QColor("#22000000");
}
QColor ThemeController::textPrimary() const {
    return m_dark ? QColor("#FFFFFF") : QColor("#1A1A1A");
}
QColor ThemeController::textSecondary() const {
    return m_dark ? QColor("#B8B8B8") : QColor("#404040");
}
QColor ThemeController::textTertiary() const {
    return m_dark ? QColor("#9AA0A6") : QColor("#606060");
}
QColor ThemeController::textMuted() const {
    return m_dark ? QColor("#7A7A7A") : QColor("#8A8A8A");
}
QColor ThemeController::accent() const {
    return m_dark ? QColor("#4CC2FF") : QColor("#0078D4");
}
QColor ThemeController::accentSoft() const {
    return m_dark ? QColor("#7FD6FF") : QColor("#005A9E");
}
QColor ThemeController::accentBadge() const {
    return m_dark ? QColor("#26333D") : QColor("#D6E8F5");
}
QColor ThemeController::artPlaceholder() const {
    return m_dark ? QColor("#333333") : QColor("#DDDDDD");
}
QColor ThemeController::track() const {
    return m_dark ? QColor("#3A3A3A") : QColor("#C8C8C8");
}
