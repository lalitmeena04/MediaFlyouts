#include <gtest/gtest.h>

#include <QSignalSpy>

#include <Windows.h>

#include "GlobalShortcutController.h"
#include "Settings.h"
#include "TestSupport/IsolatedSettings.h"

TEST(GlobalShortcutControllerTest, ReportsInvalidBindingsAndPausesDuringCapture) {
    mftest::clearUserSettings();
    Settings settings;
    GlobalShortcutController shortcuts(&settings);

    EXPECT_TRUE(shortcuts.status().contains(QStringLiteral("0 shortcut(s) active.")));
    settings.setShortcutPlayPause(QStringLiteral("Ctrl+Alt+Shift"));
    EXPECT_TRUE(shortcuts.status().contains(QStringLiteral("unsupported shortcut")));

    shortcuts.setCaptureMode(true);
    EXPECT_TRUE(shortcuts.status().contains(QStringLiteral("paused")));
    shortcuts.setCaptureMode(false);
    EXPECT_TRUE(shortcuts.status().contains(QStringLiteral("unsupported shortcut")));
}

TEST(GlobalShortcutControllerTest, HandlesHotkeyWhenResultPointerIsNull) {
    mftest::clearUserSettings();
    Settings settings;
    settings.setShortcutPlayPause(QStringLiteral("Ctrl+Alt+Shift+F24"));
    GlobalShortcutController shortcuts(&settings);
    if(!shortcuts.status().contains(QStringLiteral("1 shortcut(s) active.")))
        GTEST_SKIP() << shortcuts.status().toStdString();

    QSignalSpy triggered(&shortcuts, &GlobalShortcutController::actionTriggered);
    ASSERT_TRUE(triggered.isValid());
    MSG message{};
    message.message = WM_HOTKEY;
    message.wParam = 0x4D00;

    EXPECT_TRUE(shortcuts.nativeEventFilter(QByteArray(), &message, nullptr));
    ASSERT_EQ(triggered.size(), 1);
    EXPECT_EQ(triggered.takeFirst().at(0).toString(), QStringLiteral("playPause"));

    qintptr result = 1;
    EXPECT_TRUE(shortcuts.nativeEventFilter(QByteArray(), &message, &result));
    EXPECT_EQ(result, 0);
}

TEST(GlobalShortcutControllerTest, FocusShortcutCanChangeSettingsWhileDispatching) {
    mftest::clearUserSettings();
    Settings settings;
    settings.setShortcutToggleFocusMode(QStringLiteral("Ctrl+Alt+Shift+F24"));
    GlobalShortcutController shortcuts(&settings);
    if(!shortcuts.status().contains(QStringLiteral("1 shortcut(s) active.")))
        GTEST_SKIP() << shortcuts.status().toStdString();

    QObject::connect(&shortcuts, &GlobalShortcutController::actionTriggered, &shortcuts, [&settings](const QString& action) {
        if(action == QStringLiteral("toggleFocusMode"))
            settings.setFocusMode(!settings.focusMode());
    });

    MSG message{};
    message.message = WM_HOTKEY;
    message.wParam = 0x4D06;
    qintptr result = 1;

    EXPECT_TRUE(shortcuts.nativeEventFilter(QByteArray(), &message, &result));
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(settings.focusMode());
}
