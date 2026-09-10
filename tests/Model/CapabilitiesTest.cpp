#include <gtest/gtest.h>

#include <QSignalSpy>

#include "Capabilities.h"

// Capabilities start available. Subsystem failure clears them.
TEST(CapabilitiesTest, DefaultsToAvailable) {
    Capabilities caps;
    EXPECT_TRUE(caps.audioAvailable());
    EXPECT_TRUE(caps.mediaSessionAvailable());
    EXPECT_TRUE(caps.lockKeysAvailable());
    EXPECT_TRUE(caps.mediaKeysAvailable());
    EXPECT_TRUE(caps.chargerAvailable());
}

TEST(CapabilitiesTest, SettersUpdateEachFlagIndependently) {
    Capabilities caps;

    caps.setAudioAvailable(false);
    EXPECT_FALSE(caps.audioAvailable());
    EXPECT_TRUE(caps.mediaSessionAvailable());
    EXPECT_TRUE(caps.lockKeysAvailable());
    EXPECT_TRUE(caps.mediaKeysAvailable());

    caps.setMediaSessionAvailable(false);
    caps.setLockKeysAvailable(false);
    caps.setMediaKeysAvailable(false);
    caps.setChargerAvailable(false);
    EXPECT_FALSE(caps.mediaSessionAvailable());
    EXPECT_FALSE(caps.lockKeysAvailable());
    EXPECT_FALSE(caps.mediaKeysAvailable());
    EXPECT_FALSE(caps.chargerAvailable());
}

TEST(CapabilitiesTest, EmitsChangedOnlyOnTransition) {
    Capabilities caps;
    QSignalSpy spy(&caps, &Capabilities::changed);

    caps.setAudioAvailable(true); // no-op, already available
    EXPECT_EQ(spy.count(), 0);

    caps.setAudioAvailable(false);
    EXPECT_EQ(spy.count(), 1);

    caps.setAudioAvailable(false); // no-op, unchanged
    EXPECT_EQ(spy.count(), 1);

    caps.setAudioAvailable(true);
    EXPECT_EQ(spy.count(), 2);
}
