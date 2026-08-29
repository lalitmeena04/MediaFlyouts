#pragma once

#include <windows.h>

#include <hidsdi.h>
#include <hidpi.h>

#include <QString>

#include <vector>

// One HID top-level collection, as needed by the vendor battery providers.
struct HidCollection {
        QString path;
        USHORT vendorId = 0;
        USHORT productId = 0;
        USAGE usagePage = 0;
        USAGE usage = 0;
        USHORT inputLength = 0;
        USHORT outputLength = 0;
        USHORT featureLength = 0;
        QString product;
};

// Lists the HID collections of one vendor. Opened with no access rights, which is all the
// descriptor queries need and is the only way system mouse/keyboard collections can be
// opened at all.
std::vector<HidCollection> enumerateHidCollections(USHORT vendorId);

// Maps a Generic Desktop usage onto our icon categories ("mouse", "keyboard", "gamepad").
// Returns an empty string for pages/usages that don't identify a device type.
QString categoryFromHidUsage(USAGE usagePage, USAGE usage);

// Best device type for a product across all its collections. Gaming mice also expose a
// keyboard collection for their macro keys, so the more specific type wins.
QString categoryForProduct(const std::vector<HidCollection>& collections, USHORT productId);
