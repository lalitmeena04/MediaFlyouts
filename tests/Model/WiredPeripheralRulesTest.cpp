#include <gtest/gtest.h>

#include <algorithm>

#include "WiredPeripheralRules.h"

// Storage must never reach a flyout or the widget.
TEST(WiredPeripheralRulesTest, RejectsDataStorageClasses) {
    for(const char* storage : {"USBSTOR", "DiskDrive", "Volume", "VolumeSnapshot", "CDROM", "FloppyDisk",
                               "SCSIAdapter", "TapeDrive", "MTD", "WPD"}) {
        EXPECT_TRUE(mf::isStorageDeviceClass(QString::fromLatin1(storage))) << storage;
        EXPECT_FALSE(mf::isAnnounceableWiredDevice(QString::fromLatin1(storage), QStringLiteral("SanDisk Ultra")))
            << storage;
    }
}

TEST(WiredPeripheralRulesTest, StorageClassMatchIgnoresCaseAndPadding) {
    EXPECT_TRUE(mf::isStorageDeviceClass(QStringLiteral("usbstor")));
    EXPECT_TRUE(mf::isStorageDeviceClass(QStringLiteral("  DiskDrive  ")));
}

TEST(WiredPeripheralRulesTest, RejectsHubsAndOtherPlumbing) {
    for(const char* plumbing : {"USB", "USBFN", "System", "Computer", "HDC", "Processor", "SoftwareDevice",
                                "SoftwareComponent", "Bluetooth"}) {
        EXPECT_TRUE(mf::isInfrastructureDeviceClass(QString::fromLatin1(plumbing))) << plumbing;
        EXPECT_FALSE(mf::isAnnounceableWiredDevice(QString::fromLatin1(plumbing), QStringLiteral("USB Hub")))
            << plumbing;
    }
}

// The host radio is not a peripheral. Its paired devices arrive over Bluetooth.
TEST(WiredPeripheralRulesTest, RejectsTheHostBluetoothRadio) {
    EXPECT_FALSE(mf::isAnnounceableWiredDevice(QStringLiteral("Bluetooth"),
                                               QStringLiteral("Intel(R) Wireless Bluetooth(R)")));
}

TEST(WiredPeripheralRulesTest, AcceptsRealPeripherals) {
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("Keyboard"), QStringLiteral("AULA F75")));
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("Mouse"), QStringLiteral("Razer Basilisk V3 X")));
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("Media"), QStringLiteral("Logitech G435")));
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("HIDClass"), QStringLiteral("Stream Deck")));
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("Printer"), QStringLiteral("HP LaserJet")));
    EXPECT_TRUE(mf::isAnnounceableWiredDevice(QStringLiteral("Camera"), QStringLiteral("Logitech C920")));
}

// Windows falls back to placeholder text when a device reports no product string.
TEST(WiredPeripheralRulesTest, RejectsPlaceholderNames) {
    for(const char* generic : {"USB Input Device", "USB Composite Device", "USB Audio Device", "Generic USB Hub",
                               "USB Root Hub", "Unknown Device", "HID-compliant device",
                               "Generic Billboard Device"}) {
        EXPECT_TRUE(mf::isGenericDeviceName(QString::fromLatin1(generic))) << generic;
        EXPECT_FALSE(mf::isAnnounceableWiredDevice(QStringLiteral("HIDClass"), QString::fromLatin1(generic)))
            << generic;
    }
    EXPECT_FALSE(mf::isGenericDeviceName(QStringLiteral("Razer BlackWidow")));
}

TEST(WiredPeripheralRulesTest, RejectsEmptyNames) {
    EXPECT_FALSE(mf::isAnnounceableWiredDevice(QStringLiteral("Keyboard"), QString()));
    EXPECT_FALSE(mf::isAnnounceableWiredDevice(QStringLiteral("Keyboard"), QStringLiteral("   ")));
}

TEST(WiredPeripheralRulesTest, MapsSetupClassToCategory) {
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("Keyboard")), QStringLiteral("keyboard"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("Mouse")), QStringLiteral("mouse"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("XnaComposite")), QStringLiteral("gamepad"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("XboxComposite")), QStringLiteral("gamepad"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("Media")), QStringLiteral("headset"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("AudioEndpoint")), QStringLiteral("headset"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("Camera")), QStringLiteral("camera"));
    EXPECT_EQ(mf::categoryFromDeviceClass(QStringLiteral("Image")), QStringLiteral("camera"));
}

// A webcam also exposes a microphone. Enumeration order must not pick the icon.
TEST(WiredPeripheralRulesTest, CameraOutranksHeadsetWhenNodesMerge) {
    const QStringList webcam{QStringLiteral("headset"), QStringLiteral("camera")};
    EXPECT_EQ(mf::mergedCategory(webcam), QStringLiteral("camera"));

    QStringList reversed = webcam;
    std::reverse(reversed.begin(), reversed.end());
    EXPECT_EQ(mf::mergedCategory(reversed), QStringLiteral("camera"));
}

TEST(WiredPeripheralRulesTest, MergedCategoryIgnoresEmptyEntries) {
    EXPECT_EQ(mf::mergedCategory({QString(), QStringLiteral("keyboard"), QString()}),
              QStringLiteral("keyboard"));
    EXPECT_TRUE(mf::mergedCategory({QString(), QStringLiteral("   ")}).isEmpty());
    EXPECT_TRUE(mf::mergedCategory({}).isEmpty());
}

// An unmapped class still names the device, so keep whatever the caller found.
TEST(WiredPeripheralRulesTest, MergedCategoryKeepsUnrankedValues) {
    EXPECT_EQ(mf::mergedCategory({QStringLiteral("printer")}), QStringLiteral("printer"));
}

TEST(WiredPeripheralRulesTest, UnknownClassHasNoCategory) {
    EXPECT_TRUE(mf::categoryFromDeviceClass(QStringLiteral("HIDClass")).isEmpty());
    EXPECT_TRUE(mf::categoryFromDeviceClass(QString()).isEmpty());
}

// The dongle is not the peripheral. Only the device it pairs with is listed.
TEST(WiredPeripheralRulesTest, RejectsWirelessReceivers) {
    for(const char* receiver : {"2.4G Wireless Receiver", "USB Receiver", "Logitech Unifying Receiver",
                                "Wireless Dongle", "2.4 GHz Receiver"}) {
        EXPECT_TRUE(mf::isWirelessReceiverName(QString::fromLatin1(receiver))) << receiver;
        EXPECT_FALSE(mf::isAnnounceableWiredDevice(QStringLiteral("HIDClass"), QString::fromLatin1(receiver)))
            << receiver;
    }
    EXPECT_FALSE(mf::isWirelessReceiverName(QStringLiteral("Razer Basilisk V3 X")));
}

TEST(WiredPeripheralRulesTest, ReadsVendorAndProductFromInstanceId) {
    quint16 vendorId = 0;
    quint16 productId = 0;

    ASSERT_TRUE(mf::parseUsbVendorProduct(QStringLiteral("USB\\VID_046D&PID_0ADF\\5&1234ABCD&0&2"), vendorId,
                                          productId));
    EXPECT_EQ(vendorId, 0x046D);
    EXPECT_EQ(productId, 0x0ADF);

    // Windows is not consistent about case in instance ids.
    ASSERT_TRUE(mf::parseUsbVendorProduct(QStringLiteral("usb\\vid_3554&pid_fa09\\6&abc"), vendorId, productId));
    EXPECT_EQ(vendorId, 0x3554);
    EXPECT_EQ(productId, 0xFA09);
}

TEST(WiredPeripheralRulesTest, RejectsInstanceIdsWithoutVendorOrProduct) {
    quint16 vendorId = 0;
    quint16 productId = 0;
    EXPECT_FALSE(mf::parseUsbVendorProduct(QStringLiteral("HID\\VID_046D"), vendorId, productId));
    EXPECT_FALSE(mf::parseUsbVendorProduct(QStringLiteral("USB\\VID_ZZZZ&PID_0ADF\\1"), vendorId, productId));
    EXPECT_FALSE(mf::parseUsbVendorProduct(QString(), vendorId, productId));
}

// One physical unit reports several function names. The user wants one row.
TEST(WiredPeripheralRulesTest, MergesFunctionNamesOfOneDevice) {
    EXPECT_EQ(mf::mergedDeviceName({QStringLiteral("Owl Lite Camera"), QStringLiteral("Owl Lite Audio")}),
              QStringLiteral("Owl Lite"));
    // A name that fully contains the other stays whole.
    EXPECT_EQ(mf::mergedDeviceName({QStringLiteral("Gaming Keyboard"),
                                    QStringLiteral("Gaming Keyboard Consumer Control")}),
              QStringLiteral("Gaming Keyboard"));
    EXPECT_EQ(mf::mergedDeviceName({QStringLiteral("Razer Basilisk V3 X"), QStringLiteral("Razer Basilisk V3 X")}),
              QStringLiteral("Razer Basilisk V3 X"));
}

TEST(WiredPeripheralRulesTest, KeepsFirstNameWhenFunctionsShareNothing) {
    EXPECT_EQ(mf::mergedDeviceName({QStringLiteral("Owl Lite Camera"), QStringLiteral("Some Microphone")}),
              QStringLiteral("Owl Lite Camera"));
}

TEST(WiredPeripheralRulesTest, MergeHandlesEmptyAndSingleInput) {
    EXPECT_TRUE(mf::mergedDeviceName({}).isEmpty());
    EXPECT_TRUE(mf::mergedDeviceName({QStringLiteral("   ")}).isEmpty());
    EXPECT_EQ(mf::mergedDeviceName({QStringLiteral("  Stream Deck  ")}), QStringLiteral("Stream Deck"));
}
