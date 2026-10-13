#pragma once

#include <windows.h>

#include <hidsdi.h>
#include <hidpi.h>

#include <QString>

#include <vector>

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

std::vector<HidCollection> enumerateHidCollections(USHORT vendorId);

QString categoryFromHidUsage(USAGE usagePage, USAGE usage);

QString categoryForProduct(const std::vector<HidCollection>& collections, USHORT productId);
