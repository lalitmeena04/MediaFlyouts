#include <gtest/gtest.h>

#include "BatteryProviders/WirelessReceiverProvider.h"

TEST(WirelessReceiverProviderTest, RecognizesKnownPresenceOnlyReceivers) {
    const WirelessReceiverProfile* g435 = wirelessReceiverProfile(0x046D, 0x0ADF);
    ASSERT_NE(g435, nullptr);
    EXPECT_EQ(g435->name, QStringLiteral("Logitech G435"));
    EXPECT_EQ(g435->category, QStringLiteral("headset"));

    const WirelessReceiverProfile* aula = wirelessReceiverProfile(0x3554, 0xFA09);
    ASSERT_NE(aula, nullptr);
    EXPECT_EQ(aula->name, QStringLiteral("AULA wireless keyboard"));
    EXPECT_EQ(aula->category, QStringLiteral("keyboard"));
}

TEST(WirelessReceiverProviderTest, RejectsUnsupportedAndWiredDeviceIds) {
    EXPECT_EQ(wirelessReceiverProfile(0x258A, 0x010C), nullptr);
    EXPECT_EQ(wirelessReceiverProfile(0x1A2C, 0x9407), nullptr);
    EXPECT_EQ(wirelessReceiverProfile(0x046D, 0xC539), nullptr);
}
