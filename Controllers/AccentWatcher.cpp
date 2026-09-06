#include "AccentWatcher.h"

#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt::Windows::UI::ViewManagement;

struct AccentWatcher::Impl {
        UISettings settings{nullptr};
};

AccentWatcher::AccentWatcher(QObject* parent) : QObject(parent), m_impl(std::make_unique<Impl>()) {
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &AccentWatcher::refresh);
}

AccentWatcher::~AccentWatcher() = default;

QColor AccentWatcher::accent() const {
    return m_accent;
}

void AccentWatcher::start() {
    m_impl->settings = UISettings();
    refresh();
    // Poll accent changes. WinRT event handler is hard to link.
    m_timer.start();
}

void AccentWatcher::refresh() {
    if(!m_impl->settings)
        return;
    const auto c = m_impl->settings.GetColorValue(UIColorType::Accent);
    const QColor color(c.R, c.G, c.B, c.A);
    if(color == m_accent)
        return;
    m_accent = color;
    emit accentChanged(m_accent);
}
