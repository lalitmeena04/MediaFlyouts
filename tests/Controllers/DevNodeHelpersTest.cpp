#include <gtest/gtest.h>

#include "BatteryProviders/DevNodeHelpers.h"

#include <initguid.h>
#include <devpkey.h>

// Bluetooth PnP nodes store the address as 12 hex digits.
TEST(DevNodeHelpersTest, ParsesTwelveDigitHexAddress) {
    quint64 out = 0;
    ASSERT_TRUE(parseBluetoothAddressHex(QStringLiteral("A1B2C3D4E5F6"), out));
    EXPECT_EQ(out, 0xA1B2C3D4E5F6ULL);
}

TEST(DevNodeHelpersTest, HexParseIsCaseInsensitiveAndTrimsWhitespace) {
    quint64 upper = 0, lower = 0, padded = 0;
    ASSERT_TRUE(parseBluetoothAddressHex(QStringLiteral("0123456789AB"), upper));
    ASSERT_TRUE(parseBluetoothAddressHex(QStringLiteral("0123456789ab"), lower));
    ASSERT_TRUE(parseBluetoothAddressHex(QStringLiteral("  0123456789AB "), padded));
    EXPECT_EQ(upper, lower);
    EXPECT_EQ(upper, padded);
}

TEST(DevNodeHelpersTest, RejectsMalformedHexAddress) {
    quint64 out = 0xDEAD;
    EXPECT_FALSE(parseBluetoothAddressHex(QString(), out));
    EXPECT_FALSE(parseBluetoothAddressHex(QStringLiteral("not-hex"), out));
    EXPECT_FALSE(parseBluetoothAddressHex(QStringLiteral("0123456789ABC"), out)); // 13 digits
    EXPECT_FALSE(parseBluetoothAddressHex(QStringLiteral("12:34:56:78:9A:BC"), out));
    // Output is untouched on failure.
    EXPECT_EQ(out, 0xDEADULL);
}

// A malformed address must never reach the Bluetooth API.
TEST(DevNodeHelpersTest, ConnectedCheckRejectsBadHexWithoutQuerying) {
    EXPECT_FALSE(bluetoothAddressConnected(QString()));
    EXPECT_FALSE(bluetoothAddressConnected(QStringLiteral("zz")));
}

// Address zero is never a paired device.
TEST(DevNodeHelpersTest, ConnectedCheckIsFalseForNullAddress) {
    EXPECT_FALSE(bluetoothAddressConnected(quint64{0}));
}

// CM_* calls on a bogus DEVINST must fail closed, not crash.
TEST(DevNodeHelpersTest, InvalidDevInstFailsClosed) {
    constexpr DEVINST kBogus = 0xFFFFFFF0u;
    EXPECT_TRUE(devNodeStringProperty(kBogus, DEVPKEY_Device_FriendlyName).isEmpty());
    EXPECT_TRUE(devNodeContainerId(kBogus).isEmpty());
    EXPECT_FALSE(devNodeIsActive(kBogus));
}
