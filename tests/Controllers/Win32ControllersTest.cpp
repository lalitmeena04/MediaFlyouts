#include <gtest/gtest.h>

#include <QVariant>

#include "AudioCapture.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"

// These Windows-only controllers wrap COM / DWM APIs; without a device or window
// we can still verify their construction and out-of-range operations are safe.

TEST(Win32ControllersTest, AudioCaptureStartsSilentAndInactive) {
    AudioCapture capture(24);
    EXPECT_FALSE(capture.active());
    EXPECT_EQ(capture.bands().size(), 24);
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
    effects.applyAcrylic(nullptr, true);
    effects.applyAcrylic(nullptr, false);
    effects.setDarkFrame(nullptr, true);
    SUCCEED();
}
