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

// List HID collections for one vendor.
// Open with no access rights for descriptor queries.
std::vector<HidCollection> enumerateHidCollections(USHORT vendorId);

// Map Generic Desktop usage to icon category.
// Return empty string for unknown page or usage.
QString categoryFromHidUsage(USAGE usagePage, USAGE usage);

// Best device type for a product across all collections.
// More specific type wins.
QString categoryForProduct(const std::vector<HidCollection>& collections, USHORT productId);
