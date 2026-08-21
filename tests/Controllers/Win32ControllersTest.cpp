#include <gtest/gtest.h>

#include <QVariant>
#include <QVariantMap>

#include <windows.h>

#include <objbase.h>

#include "AudioCapture.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"

// These Windows-only controllers wrap COM / DWM APIs; without a device or window
// we can still verify their construction and out-of-range operations are safe.

TEST(Win32ControllersTest, AudioCaptureStartsSilentAndInactive) {
    AudioCapture capture(24);
    EXPECT_FALSE(capture.active());
    EXPECT_EQ(capture.bands().size(), 24);
    EXPECT_DOUBLE_EQ(capture.level(), 0.0);
    for(const QVariant& band : capture.bands())
        EXPECT_DOUBLE_EQ(band.toDouble(), 0.0);
}

TEST(Win32ControllersTest, VolumeMixerDefaultsEmptyAndIgnoresBadIndices) {
    VolumeMixer mixer;
    EXPECT_TRUE(mixer.sessions().isEmpty());
    mixer.setSessionVolume(0, 50);
    mixer.setSessionVolume(-1, 10);
    mixer.setSessionMute(3, true);
    EXPECT_TRUE(mixer.sessions().isEmpty());
}

TEST(Win32ControllersTest, WindowEffectsToleratesNullWindow) {
    WindowEffects effects;
    effects.setDarkFrame(nullptr, true);
    effects.setDarkFrame(nullptr, false);
    SUCCEED();
}

// With COM initialised we can exercise the real enumeration / capture code paths.
// These degrade gracefully whether or not the CI runner exposes an audio device.
class Win32ComTest : public ::testing::Test {
    protected:
        void SetUp() override {
            CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        }
        void TearDown() override {
            CoUninitialize();
        }
};

TEST_F(Win32ComTest, VolumeMixerRefreshProducesValidSessions) {
    VolumeMixer mixer;
    mixer.refresh();
    for(const QVariant& entry : mixer.sessions()) {
        const QVariantMap m = entry.toMap();
        EXPECT_GE(m.value(QStringLiteral("volume")).toInt(), 0);
        EXPECT_LE(m.value(QStringLiteral("volume")).toInt(), 100);
        EXPECT_FALSE(m.value(QStringLiteral("name")).toString().isEmpty());
    }
}

TEST_F(Win32ComTest, AudioCaptureStartStopIsSafe) {
    AudioCapture capture(16);
    capture.start();
    capture.stop();
    EXPECT_FALSE(capture.active());
    EXPECT_EQ(capture.bands().size(), 16);
}
