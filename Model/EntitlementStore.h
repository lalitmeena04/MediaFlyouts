#pragma once

#include <memory>

#include <QByteArray>
#include <QString>

#include "Entitlement.h"

namespace mf {

class IEntitlementCodec {
    public:
        virtual ~IEntitlementCodec() = default;
        // Written in the file header. A file from another codec is rejected.
        virtual char id() const = 0;
        virtual QString name() const = 0;
        virtual QByteArray seal(const QByteArray& plain) const = 0;
        virtual bool open(const QByteArray& sealed, QByteArray* plain) const = 0;
};

// Salted SHA-256 tag. Detects edits, not copies.
class ChecksumCodec : public IEntitlementCodec {
    public:
        char id() const override;
        QString name() const override;
        QByteArray seal(const QByteArray& plain) const override;
        bool open(const QByteArray& sealed, QByteArray* plain) const override;
};

#ifdef Q_OS_WIN
// DPAPI, bound to the signed-in Windows user. No key ships in the app.
class DpapiCodec : public IEntitlementCodec {
    public:
        char id() const override;
        QString name() const override;
        QByteArray seal(const QByteArray& plain) const override;
        bool open(const QByteArray& sealed, QByteArray* plain) const override;
};
#endif

std::unique_ptr<IEntitlementCodec> defaultEntitlementCodec();
QString defaultEntitlementFile();

QByteArray serializeEntitlementCache(const EntitlementCache& cache);
bool parseEntitlementCache(const QByteArray& json, EntitlementCache* cache);

// Own file, apart from Settings: export, import and reset never touch it.
class EntitlementStore {
    public:
        EntitlementStore();
        EntitlementStore(const QString& filePath, std::unique_ptr<IEntitlementCodec> codec);
        EntitlementStore(EntitlementStore&&) noexcept;
        EntitlementStore& operator=(EntitlementStore&&) noexcept;
        ~EntitlementStore();

        EntitlementCache load() const;
        bool save(const EntitlementCache& cache);
        void clear();

        QString fileName() const;
        QString codecName() const;

    private:
        QString m_file;
        std::unique_ptr<IEntitlementCodec> m_codec;
};

} // namespace mf