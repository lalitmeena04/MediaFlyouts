#pragma once

#include <windows.h>

#include <hidsdi.h>
#include <hidpi.h>

#include <QString>

#include <functional>
#include <mutex>
#include <optional>
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

class HidCollectionCache {
    public:
        using Loader = std::function<std::vector<HidCollection>()>;

        explicit HidCollectionCache(Loader loader);

        std::vector<HidCollection> forVendor(USHORT vendorId);
        void invalidate();

    private:
        Loader m_loader;
        std::mutex m_mutex;
        std::optional<std::vector<HidCollection>> m_inventory;
};

std::vector<HidCollection> enumerateHidCollections(USHORT vendorId);
void invalidateHidCollectionCache();

QString categoryFromHidUsage(USAGE usagePage, USAGE usage);

QString categoryForProduct(const std::vector<HidCollection>& collections, USHORT productId);
