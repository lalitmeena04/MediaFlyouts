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

TEST(CapabilitiesTest, ReportsNoDegradationWhileEverySubsystemWorks) {
    Capabilities caps;
    EXPECT_FALSE(caps.degraded());
    EXPECT_TRUE(caps.unavailableNames().isEmpty());
    EXPECT_EQ(caps.statusSummary(), QStringLiteral("MediaFlyouts"));
}

TEST(CapabilitiesTest, ListsEveryDownSubsystemByName) {
    Capabilities caps;
    caps.setAudioAvailable(false);
    caps.setLockKeysAvailable(false);

    EXPECT_TRUE(caps.degraded());
    EXPECT_EQ(caps.unavailableNames(), QStringList({QStringLiteral("Audio devices"), QStringLiteral("Lock keys")}));
    EXPECT_EQ(caps.statusSummary(), QStringLiteral("MediaFlyouts\nUnavailable: Audio devices, Lock keys"));
}

// Desktops have no battery. That is normal, not a failure.
TEST(CapabilitiesTest, MissingChargerDoesNotCountAsDegraded) {
    Capabilities caps;
    caps.setChargerAvailable(false);

    EXPECT_FALSE(caps.degraded());
    EXPECT_TRUE(caps.unavailableNames().isEmpty());
    EXPECT_EQ(caps.statusSummary(), QStringLiteral("MediaFlyouts"));
}

TEST(CapabilitiesTest, ClearsDegradationWhenSubsystemRecovers) {
    Capabilities caps;
    caps.setMediaSessionAvailable(false);
    EXPECT_TRUE(caps.degraded());

    caps.setMediaSessionAvailable(true);
    EXPECT_FALSE(caps.degraded());
    EXPECT_TRUE(caps.unavailableNames().isEmpty());
}
