#pragma once

#include <memory>

#include <QByteArray>
#include <QString>

#include "Entitlement.h"

namespace mf {

// Seals the cache bytes before they reach the disk and opens them again.
class IEntitlementCodec {
    public:
        virtual ~IEntitlementCodec() = default;
        // One byte written in the file header. A file sealed by another codec is rejected.
        virtual char id() const = 0;
        virtual QString name() const = 0;
        // Empty on failure.
        virtual QByteArray seal(const QByteArray& plain) const = 0;
        // False when the bytes were altered or sealed for someone else.
        virtual bool open(const QByteArray& sealed, QByteArray* plain) const = 0;
};

// Portable. SHA-256 tag in front of the plain bytes. Detects edits, not copies.
class ChecksumCodec : public IEntitlementCodec {
    public:
        char id() const override;
        QString name() const override;
        QByteArray seal(const QByteArray& plain) const override;
        bool open(const QByteArray& sealed, QByteArray* plain) const override;
};

#ifdef Q_OS_WIN
// Windows DPAPI. The OS holds the key, bound to the signed-in Windows user.
// Another user or another PC cannot open the file. Edits fail the built-in check.
class DpapiCodec : public IEntitlementCodec {
    public:
        char id() const override;
        QString name() const override;
        QByteArray seal(const QByteArray& plain) const override;
        bool open(const QByteArray& sealed, QByteArray* plain) const override;
};
#endif

// DPAPI on Windows, checksum elsewhere.
std::unique_ptr<IEntitlementCodec> defaultEntitlementCodec();
// "Entitlement.dat" next to the user settings ini.
QString defaultEntitlementFile();

// Compact JSON. Times are UTC ISO strings.
QByteArray serializeEntitlementCache(const EntitlementCache& cache);
// False on malformed input. `cache` is untouched then.
bool parseEntitlementCache(const QByteArray& json, EntitlementCache* cache);

// Keeps the last EntitlementCache in one sealed file, apart from the user
// settings. Settings export, import and reset never touch it.
class EntitlementStore {
    public:
        EntitlementStore();
        EntitlementStore(const QString& filePath, std::unique_ptr<IEntitlementCodec> codec);
        EntitlementStore(EntitlementStore&&) noexcept;
        EntitlementStore& operator=(EntitlementStore&&) noexcept;
        ~EntitlementStore();

        // Invalid cache when the file is missing, altered, or sealed by another codec or user.
        EntitlementCache load() const;
        // False when the file could not be written.
        bool save(const EntitlementCache& cache);
        void clear();

        QString fileName() const;
        QString codecName() const;

    private:
        QString m_file;
        std::unique_ptr<IEntitlementCodec> m_codec;
};

} // namespace mf
