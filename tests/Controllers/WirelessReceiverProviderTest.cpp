#include <gtest/gtest.h>

#include "BatteryProviders/WirelessReceiverProvider.h"

TEST(WirelessReceiverProviderTest, RecognizesKnownPresenceOnlyReceivers) {
    const mf::ReceiverProfile* g435 = wirelessReceiverProfile(0x046D, 0x0ADF);
    ASSERT_NE(g435, nullptr);
    EXPECT_EQ(g435->name, QStringLiteral("Logitech G435"));
    EXPECT_EQ(g435->category, QStringLiteral("headset"));

    const mf::ReceiverProfile* aula = wirelessReceiverProfile(0x3554, 0xFA09);
    ASSERT_NE(aula, nullptr);
    EXPECT_EQ(aula->name, QStringLiteral("AULA wireless keyboard"));
    EXPECT_EQ(aula->category, QStringLiteral("keyboard"));
}

TEST(WirelessReceiverProviderTest, RejectsUnsupportedAndWiredDeviceIds) {
    EXPECT_EQ(wirelessReceiverProfile(0x258A, 0x010C), nullptr);
    EXPECT_EQ(wirelessReceiverProfile(0x1A2C, 0x9407), nullptr);
    EXPECT_EQ(wirelessReceiverProfile(0x046D, 0xC539), nullptr);
}

// The AULA keyboard enumerates under other ids on its cable.
// Its dongle stays plugged in, so the cable has to win.
TEST(WirelessReceiverProviderTest, KnowsTheWiredTwinOfTheAulaKeyboard) {
    const mf::ReceiverProfile* aula = wirelessReceiverProfile(0x3554, 0xFA09);
    ASSERT_NE(aula, nullptr);
    EXPECT_EQ(aula->wiredVendorId, 0x258A);
    EXPECT_EQ(aula->wiredProductId, 0x010C);
}

// A receiver with no cable mode must not accidentally match a device at 0000:0000.
TEST(WirelessReceiverProviderTest, ReceiversWithoutACableModeDeclareNoTwin) {
    const mf::ReceiverProfile* g435 = wirelessReceiverProfile(0x046D, 0x0ADF);
    ASSERT_NE(g435, nullptr);
    EXPECT_EQ(g435->wiredVendorId, 0);
    EXPECT_EQ(g435->wiredProductId, 0);
}
