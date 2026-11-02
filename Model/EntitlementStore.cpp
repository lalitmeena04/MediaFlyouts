#include "EntitlementStore.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>

#include <wincrypt.h>
#endif

#include "Logging.h"

namespace mf {

namespace {

// File layout: magic, one codec id byte, sealed payload.
constexpr char kMagic[] = {'M', 'F', 'E', '1'};
constexpr int kMagicSize = 4;
constexpr int kHeaderSize = kMagicSize + 1;
// Mixed into every seal. Bump when the payload layout changes so old files are dropped.
constexpr char kSalt[] = "MediaFlyouts.Entitlement.v1";
constexpr int kSaltSize = sizeof(kSalt) - 1;
constexpr int kSha256Size = 32;

constexpr char kTierKey[] = "tier";
constexpr char kTrialEndKey[] = "trialEnd";
constexpr char kLastSeenKey[] = "lastSeen";

QString timeToText(const QDateTime& t) {
    return t.isValid() ? t.toUTC().toString(Qt::ISODateWithMs) : QString();
}

// False when `text` is present but not a time. Empty text is a valid "no time".
bool timeFromText(const QString& text, QDateTime* out) {
    if(text.isEmpty()) {
        *out = QDateTime();
        return true;
    }
    // Stored text ends in "Z", so the parsed value is already UTC.
    const QDateTime t = QDateTime::fromString(text, Qt::ISODateWithMs);
    if(!t.isValid())
        return false;
    *out = t;
    return true;
}

bool tierFromId(const QString& id, Tier* tier) {
    for(Tier t : {Tier::Free, Tier::Trial, Tier::Pro}) {
        if(tierId(t) == id) {
            *tier = t;
            return true;
        }
    }
    return false;
}

// A missing key reads as empty. Any non-string value is malformed.
bool textField(const QJsonObject& o, const char* key, QString* out) {
    const QJsonValue v = o.value(QLatin1String(key));
    if(v.isUndefined()) {
        out->clear();
        return true;
    }
    if(!v.isString())
        return false;
    *out = v.toString();
    return true;
}

QByteArray saltedSha256(const QByteArray& plain) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(QByteArrayView(kSalt, kSaltSize));
    hash.addData(plain);
    return hash.result();
}

} // namespace

// --- ChecksumCodec ---------------------------------------------------------

char ChecksumCodec::id() const {
    return 'C';
}

QString ChecksumCodec::name() const {
    return QStringLiteral("checksum");
}

QByteArray ChecksumCodec::seal(const QByteArray& plain) const {
    return saltedSha256(plain) + plain;
}

bool ChecksumCodec::open(const QByteArray& sealed, QByteArray* plain) const {
    if(sealed.size() < kSha256Size)
        return false;
    const QByteArray body = sealed.mid(kSha256Size);
    if(sealed.left(kSha256Size) != saltedSha256(body))
        return false;
    *plain = body;
    return true;
}

// --- DpapiCodec ------------------------------------------------------------

#ifdef Q_OS_WIN
namespace {
constexpr wchar_t kDpapiDescription[] = L"MediaFlyouts entitlement";

DATA_BLOB saltBlob() {
    DATA_BLOB blob;
    blob.cbData = static_cast<DWORD>(kSaltSize);
    blob.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(kSalt));
    return blob;
}
} // namespace

char DpapiCodec::id() const {
    return 'D';
}

QString DpapiCodec::name() const {
    return QStringLiteral("dpapi");
}

QByteArray DpapiCodec::seal(const QByteArray& plain) const {
    DATA_BLOB in;
    in.cbData = static_cast<DWORD>(plain.size());
    in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plain.constData()));
    DATA_BLOB entropy = saltBlob();
    DATA_BLOB out{};
    if(!CryptProtectData(&in, kDpapiDescription, &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) {
        TRC_WARN("entitlement store: CryptProtectData failed (err={})", GetLastError());
        return {};
    }
    const QByteArray sealed(reinterpret_cast<const char*>(out.pbData), static_cast<qsizetype>(out.cbData));
    LocalFree(out.pbData);
    return sealed;
}

bool DpapiCodec::open(const QByteArray& sealed, QByteArray* plain) const {
    if(sealed.isEmpty())
        return false;
    DATA_BLOB in;
    in.cbData = static_cast<DWORD>(sealed.size());
    in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(sealed.constData()));
    DATA_BLOB entropy = saltBlob();
    DATA_BLOB out{};
    if(!CryptUnprotectData(&in, nullptr, &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) {
        // Expected after a file edit, or for a file from another user or PC.
        TRC_WARN("entitlement store: CryptUnprotectData failed (err={})", GetLastError());
        return false;
    }
    *plain = QByteArray(reinterpret_cast<const char*>(out.pbData), static_cast<qsizetype>(out.cbData));
    LocalFree(out.pbData);
    return true;
}
#endif

// --- defaults and payload --------------------------------------------------

std::unique_ptr<IEntitlementCodec> defaultEntitlementCodec() {
#ifdef Q_OS_WIN
    return std::make_unique<DpapiCodec>();
#else
    return std::make_unique<ChecksumCodec>();
#endif
}

QString defaultEntitlementFile() {
    // Same folder as the user settings ini. Tests redirect that folder.
    const QSettings probe(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                          QStringLiteral("MediaFlyouts"));
    return QFileInfo(probe.fileName()).absolutePath() + QStringLiteral("/Entitlement.dat");
}

QByteArray serializeEntitlementCache(const EntitlementCache& c) {
    QJsonObject o;
    o.insert(QLatin1String(kTierKey), tierId(c.tier));
    o.insert(QLatin1String(kTrialEndKey), timeToText(c.trialEnd));
    o.insert(QLatin1String(kLastSeenKey), timeToText(c.lastSeen));
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

bool parseEntitlementCache(const QByteArray& json, EntitlementCache* cache) {
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
        return false;
    const QJsonObject o = doc.object();
    EntitlementCache c;
    QString tier, trialEnd, lastSeen;
    if(!textField(o, kTierKey, &tier) || !textField(o, kTrialEndKey, &trialEnd)
       || !textField(o, kLastSeenKey, &lastSeen))
        return false;
    if(!tierFromId(tier, &c.tier))
        return false;
    if(!timeFromText(trialEnd, &c.trialEnd) || !timeFromText(lastSeen, &c.lastSeen))
        return false;
    c.valid = true;
    *cache = c;
    return true;
}

// --- EntitlementStore ------------------------------------------------------

EntitlementStore::EntitlementStore() : EntitlementStore(defaultEntitlementFile(), defaultEntitlementCodec()) {}

EntitlementStore::EntitlementStore(const QString& filePath, std::unique_ptr<IEntitlementCodec> codec)
    : m_file(filePath), m_codec(std::move(codec)) {
    if(!m_codec)
        m_codec = std::make_unique<ChecksumCodec>();
}

EntitlementStore::EntitlementStore(EntitlementStore&&) noexcept = default;
EntitlementStore& EntitlementStore::operator=(EntitlementStore&&) noexcept = default;
EntitlementStore::~EntitlementStore() = default;

EntitlementCache EntitlementStore::load() const {
    QFile file(m_file);
    if(!file.exists()) {
        TRC_DEBUG("entitlement store: no file ({})", m_file.toStdString());
        return {};
    }
    if(!file.open(QIODevice::ReadOnly)) {
        TRC_WARN("entitlement store: cannot read {}", m_file.toStdString());
        return {};
    }
    const QByteArray bytes = file.readAll();
    if(bytes.size() < kHeaderSize || bytes.left(kMagicSize) != QByteArray(kMagic, kMagicSize)) {
        TRC_WARN("entitlement store: bad header, dropping cache");
        return {};
    }
    if(bytes.at(kMagicSize) != m_codec->id()) {
        TRC_WARN("entitlement store: file codec '{}' differs from '{}', dropping cache", bytes.at(kMagicSize),
                 m_codec->name().toStdString());
        return {};
    }
    QByteArray plain;
    if(!m_codec->open(bytes.mid(kHeaderSize), &plain)) {
        TRC_WARN("entitlement store: {} rejected the file, dropping cache", m_codec->name().toStdString());
        return {};
    }
    EntitlementCache c;
    if(!parseEntitlementCache(plain, &c)) {
        TRC_WARN("entitlement store: malformed payload, dropping cache");
        return {};
    }
    TRC_DEBUG("entitlement store: loaded tier={} trialEnd={} lastSeen={}", tierId(c.tier).toStdString(),
              timeToText(c.trialEnd).toStdString(), timeToText(c.lastSeen).toStdString());
    return c;
}

bool EntitlementStore::save(const EntitlementCache& c) {
    if(!c.valid) {
        clear();
        return true;
    }
    const QByteArray sealed = m_codec->seal(serializeEntitlementCache(c));
    if(sealed.isEmpty()) {
        TRC_WARN("entitlement store: {} could not seal the cache", m_codec->name().toStdString());
        return false;
    }
    QDir().mkpath(QFileInfo(m_file).absolutePath());
    QSaveFile file(m_file);
    if(!file.open(QIODevice::WriteOnly)) {
        TRC_WARN("entitlement store: cannot write {}", m_file.toStdString());
        return false;
    }
    file.write(kMagic, kMagicSize);
    const char codecId = m_codec->id();
    file.write(&codecId, 1);
    file.write(sealed);
    if(!file.commit()) {
        TRC_WARN("entitlement store: commit failed for {}", m_file.toStdString());
        return false;
    }
    TRC_INFO("entitlement store: saved tier={} trialEnd={} via {}", tierId(c.tier).toStdString(),
             timeToText(c.trialEnd).toStdString(), m_codec->name().toStdString());
    return true;
}

void EntitlementStore::clear() {
    if(QFile::exists(m_file) && !QFile::remove(m_file)) {
        TRC_WARN("entitlement store: cannot remove {}", m_file.toStdString());
        return;
    }
    TRC_INFO("entitlement store: cleared");
}

QString EntitlementStore::fileName() const {
    return m_file;
}

QString EntitlementStore::codecName() const {
    return m_codec->name();
}

} // namespace mf
