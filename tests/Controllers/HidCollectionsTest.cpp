#include <gtest/gtest.h>

#include "BatteryProviders/HidCollections.h"

namespace {

HidCollection collectionOf(USHORT productId, USAGE usagePage, USAGE usage) {
    HidCollection collection;
    collection.productId = productId;
    collection.usagePage = usagePage;
    collection.usage = usage;
    return collection;
}

} // namespace

TEST(HidCollectionsTest, MapsGenericDesktopUsagesToCategories) {
    EXPECT_EQ(categoryFromHidUsage(0x01, 0x02), "mouse");
    EXPECT_EQ(categoryFromHidUsage(0x01, 0x06), "keyboard");
    EXPECT_EQ(categoryFromHidUsage(0x01, 0x04), "gamepad"); // joystick
    EXPECT_EQ(categoryFromHidUsage(0x01, 0x05), "gamepad");
}

TEST(HidCollectionsTest, IgnoresNonGenericDesktopPages) {
    EXPECT_EQ(categoryFromHidUsage(0x0C, 0x01), ""); // consumer control
    EXPECT_EQ(categoryFromHidUsage(0xFF00, 0x01), ""); // vendor defined
    EXPECT_EQ(categoryFromHidUsage(0x01, 0x80), ""); // system control
}

TEST(HidCollectionsTest, PrefersMouseOverTheKeyboardCollectionOfAGamingMouse) {
    // Basilisk exposes keyboard collection for macro keys and mouse collection.
    const std::vector<HidCollection> collections{
        collectionOf(0x00B9, 0x01, 0x06), // keyboard, enumerated first
        collectionOf(0x00B9, 0x0C, 0x01), // consumer control
        collectionOf(0x00B9, 0x01, 0x02), // mouse
    };
    EXPECT_EQ(categoryForProduct(collections, 0x00B9), "mouse");
}

TEST(HidCollectionsTest, ReportsKeyboardWhenThereIsNoMouseCollection) {
    const std::vector<HidCollection> collections{
        collectionOf(0x010C, 0x0C, 0x01),
        collectionOf(0x010C, 0x01, 0x06),
    };
    EXPECT_EQ(categoryForProduct(collections, 0x010C), "keyboard");
}

TEST(HidCollectionsTest, IgnoresCollectionsOfOtherProducts) {
    const std::vector<HidCollection> collections{
        collectionOf(0x1111, 0x01, 0x02), // a mouse, but a different product
        collectionOf(0x2222, 0x01, 0x06),
    };
    EXPECT_EQ(categoryForProduct(collections, 0x2222), "keyboard");
}

TEST(HidCollectionsTest, ReturnsEmptyWhenNothingIdentifiesTheDevice) {
    // Headsets expose vendor and audio collections only.
    const std::vector<HidCollection> collections{
        collectionOf(0x0ADF, 0xFF00, 0x01),
        collectionOf(0x0ADF, 0x0C, 0x01),
    };
    EXPECT_EQ(categoryForProduct(collections, 0x0ADF), "");
}
