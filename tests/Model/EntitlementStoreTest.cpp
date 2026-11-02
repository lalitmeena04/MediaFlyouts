#include <gtest/gtest.h>

#include <memory>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTimeZone>

#include "EntitlementStore.h"
#include "TestSupport/IsolatedSettings.h"

using mf::ChecksumCodec;
using mf::EntitlementCache;
using mf::EntitlementStore;
using mf::IEntitlementCodec;
using mf::Tier;

namespace {

EntitlementCache sampleCache() {
    EntitlementCache c;
    c.valid = true;
    c.tier = Tier::Trial;
    c.trialEnd = QDateTime(QDate(2027, 2, 4), QTime(12, 30, 15, 250), QTimeZone::UTC);
    c.lastSeen = QDateTime(QDate(2027, 1, 20), QTime(8, 0), QTimeZone::UTC);
    return c;
}

QByteArray readAll(const QString& path) {
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly))
        return {};
    return f.readAll();
}

void writeAll(const QString& path, const QByteArray& bytes) {
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write(bytes);
}

void corruptByte(const QString& path, int offset) {
    QByteArray bytes = readAll(path);
    ASSERT_GT(bytes.size(), offset);
    bytes[offset] = static_cast<char>(bytes[offset] ^ 0x01);
    writeAll(path, bytes);
}

// --- codec contract, run for every codec -----------------------------------

using CodecFactory = std::unique_ptr<IEntitlementCodec> (*)();

std::unique_ptr<IEntitlementCodec> makeChecksum() {
    return std::make_unique<ChecksumCodec>();
}

class CodecContractTest : public ::testing::TestWithParam<CodecFactory> {
    protected:
        std::unique_ptr<IEntitlementCodec> codec = GetParam()();
};

} // namespace

TEST_P(CodecContractTest, SealThenOpenRoundTrips) {
    const QByteArray plain = QByteArrayLiteral("{\"tier\":\"pro\"}");
    const QByteArray sealed = codec->seal(plain);
    ASSERT_FALSE(sealed.isEmpty());
    EXPECT_NE(sealed, plain);
    QByteArray back;
    ASSERT_TRUE(codec->open(sealed, &back));
    EXPECT_EQ(back, plain);
}

TEST_P(CodecContractTest, AlteredSealedBytesNeverOpenToADifferentPayload) {
    // DPAPI ignores a few header bytes. Altered input must fail or open to the original.
    const QByteArray plain = QByteArrayLiteral("{\"tier\":\"trial\"}");
    const QByteArray sealed = codec->seal(plain);
    ASSERT_FALSE(sealed.isEmpty());
    for(int i = 0; i < sealed.size(); i += 7) {
        QByteArray bad = sealed;
        bad[i] = static_cast<char>(bad[i] ^ 0x01);
        QByteArray out;
        if(codec->open(bad, &out))
            EXPECT_EQ(out, plain) << "byte " << i;
    }
}

TEST_P(CodecContractTest, AlteredTrailingBytesAreRejected) {
    // Tail = payload (checksum) or integrity tag (DPAPI).
    const QByteArray sealed = codec->seal(QByteArrayLiteral("{\"tier\":\"trial\"}"));
    ASSERT_GT(sealed.size(), 8);
    for(int back = 1; back <= 8; ++back) {
        QByteArray bad = sealed;
        const int i = sealed.size() - back;
        bad[i] = static_cast<char>(bad[i] ^ 0x01);
        QByteArray out;
        EXPECT_FALSE(codec->open(bad, &out)) << "byte " << i;
    }
}

TEST_P(CodecContractTest, TruncatedAndEmptyInputAreRejected) {
    const QByteArray sealed = codec->seal(QByteArrayLiteral("x"));
    QByteArray out;
    EXPECT_FALSE(codec->open(QByteArray(), &out));
    EXPECT_FALSE(codec->open(sealed.left(sealed.size() / 2), &out));
}

TEST_P(CodecContractTest, PlainTextIsNotAcceptedAsSealed) {
    QByteArray out;
    EXPECT_FALSE(codec->open(QByteArrayLiteral("{\"tier\":\"pro\",\"trialEnd\":\"\",\"lastSeen\":\"\"}"), &out));
}

TEST_P(CodecContractTest, HasAnIdAndAName) {
    EXPECT_NE(codec->id(), '\0');
    EXPECT_FALSE(codec->name().isEmpty());
}

#ifdef Q_OS_WIN
namespace {
std::unique_ptr<IEntitlementCodec> makeDpapi() {
    return std::make_unique<mf::DpapiCodec>();
}
} // namespace
INSTANTIATE_TEST_SUITE_P(AllCodecs, CodecContractTest, ::testing::Values(&makeChecksum, &makeDpapi));
#else
INSTANTIATE_TEST_SUITE_P(AllCodecs, CodecContractTest, ::testing::Values(&makeChecksum));
#endif

TEST(EntitlementCodecTest, CodecIdsDiffer) {
    ChecksumCodec checksum;
    EXPECT_EQ(checksum.id(), 'C');
#ifdef Q_OS_WIN
    mf::DpapiCodec dpapi;
    EXPECT_EQ(dpapi.id(), 'D');
    EXPECT_NE(dpapi.id(), checksum.id());
#endif
}

TEST(EntitlementCodecTest, DefaultCodecMatchesThePlatform) {
    const auto codec = mf::defaultEntitlementCodec();
    ASSERT_TRUE(codec);
#ifdef Q_OS_WIN
    EXPECT_EQ(codec->name(), QStringLiteral("dpapi"));
#else
    EXPECT_EQ(codec->name(), QStringLiteral("checksum"));
#endif
}

// --- payload ---------------------------------------------------------------

TEST(EntitlementPayloadTest, SerializeThenParseRoundTripsEveryField) {
    EntitlementCache c;
    ASSERT_TRUE(mf::parseEntitlementCache(mf::serializeEntitlementCache(sampleCache()), &c));
    EXPECT_TRUE(c.valid);
    EXPECT_EQ(c.tier, Tier::Trial);
    EXPECT_EQ(c.trialEnd, sampleCache().trialEnd);
    EXPECT_EQ(c.lastSeen, sampleCache().lastSeen);
    EXPECT_EQ(c.trialEnd.timeSpec(), Qt::UTC);
}

TEST(EntitlementPayloadTest, ProWithoutTrialEndRoundTrips) {
    EntitlementCache pro;
    pro.valid = true;
    pro.tier = Tier::Pro;
    pro.lastSeen = sampleCache().lastSeen;
    EntitlementCache c;
    ASSERT_TRUE(mf::parseEntitlementCache(mf::serializeEntitlementCache(pro), &c));
    EXPECT_EQ(c.tier, Tier::Pro);
    EXPECT_FALSE(c.trialEnd.isValid());
    EXPECT_EQ(c.lastSeen, pro.lastSeen);
}

TEST(EntitlementPayloadTest, TimesAreStoredAsUtc) {
    EntitlementCache local = sampleCache();
    local.trialEnd = local.trialEnd.toOffsetFromUtc(5 * 3600 + 1800);
    const QByteArray json = mf::serializeEntitlementCache(local);
    EXPECT_TRUE(json.contains("2027-02-04T12:30:15.250Z")) << json.toStdString();
    EXPECT_EQ(json, mf::serializeEntitlementCache(sampleCache()));
}

TEST(EntitlementPayloadTest, MalformedPayloadsAreRejectedAndLeaveTheOutputAlone) {
    EntitlementCache untouched = sampleCache();
    const QByteArray bad[] = {
        QByteArray(),
        QByteArrayLiteral("not json"),
        QByteArrayLiteral("[]"),
        QByteArrayLiteral("{}"),
        QByteArrayLiteral("{\"tier\":\"ultra\"}"),
        QByteArrayLiteral("{\"tier\":\"pro\",\"trialEnd\":\"yesterday\"}"),
        QByteArrayLiteral("{\"tier\":\"pro\",\"lastSeen\":42}"),
    };
    for(const QByteArray& json : bad) {
        EntitlementCache c = sampleCache();
        EXPECT_FALSE(mf::parseEntitlementCache(json, &c)) << json.toStdString();
        EXPECT_EQ(c.tier, untouched.tier);
        EXPECT_EQ(c.trialEnd, untouched.trialEnd);
    }
}

// --- store -----------------------------------------------------------------

namespace {

class EntitlementStoreTest : public ::testing::Test {
    protected:
        void SetUp() override {
            mftest::isolateUserSettings();
            ASSERT_TRUE(m_dir.isValid());
            m_path = m_dir.filePath(QStringLiteral("Entitlement.dat"));
        }

        EntitlementStore store() const {
            return EntitlementStore(m_path, std::make_unique<ChecksumCodec>());
        }

        QTemporaryDir m_dir;
        QString m_path;
};

} // namespace

TEST_F(EntitlementStoreTest, DefaultFileLivesNextToTheIsolatedSettingsIni) {
    const QString file = QDir::cleanPath(mf::defaultEntitlementFile());
    EXPECT_TRUE(file.startsWith(mftest::isolateUserSettings(), Qt::CaseInsensitive)) << file.toStdString();
    EXPECT_TRUE(file.endsWith(QStringLiteral("/MediaFlyouts/Entitlement.dat"), Qt::CaseInsensitive))
        << file.toStdString();
    EXPECT_FALSE(file.contains(QStringLiteral("/AppData/Roaming/"), Qt::CaseInsensitive)) << file.toStdString();
}

TEST_F(EntitlementStoreTest, DefaultStoreUsesThePlatformCodecAndRoundTrips) {
    EntitlementStore store;
    EXPECT_EQ(store.fileName(), mf::defaultEntitlementFile());
    EXPECT_EQ(store.codecName(), mf::defaultEntitlementCodec()->name());
    ASSERT_TRUE(store.save(sampleCache()));
    const EntitlementCache c = EntitlementStore().load();
    EXPECT_TRUE(c.valid);
    EXPECT_EQ(c.trialEnd, sampleCache().trialEnd);
    store.clear();
    EXPECT_FALSE(QFile::exists(store.fileName()));
}

TEST_F(EntitlementStoreTest, MissingFileLoadsAnInvalidCache) {
    const EntitlementCache c = store().load();
    EXPECT_FALSE(c.valid);
    EXPECT_FALSE(c.trialEnd.isValid());
    EXPECT_FALSE(c.lastSeen.isValid());
}

TEST_F(EntitlementStoreTest, SaveThenLoadRoundTrips) {
    ASSERT_TRUE(store().save(sampleCache()));
    const EntitlementCache c = store().load();
    EXPECT_TRUE(c.valid);
    EXPECT_EQ(c.tier, Tier::Trial);
    EXPECT_EQ(c.trialEnd, sampleCache().trialEnd);
    EXPECT_EQ(c.lastSeen, sampleCache().lastSeen);
}

TEST_F(EntitlementStoreTest, FileStartsWithMagicAndCodecIdAndHidesThePayload) {
    ASSERT_TRUE(store().save(sampleCache()));
    const QByteArray bytes = readAll(m_path);
    ASSERT_GE(bytes.size(), 5);
    EXPECT_EQ(bytes.left(4), QByteArrayLiteral("MFE1"));
    EXPECT_EQ(bytes.at(4), 'C');
}

TEST_F(EntitlementStoreTest, AlteredPayloadIsDropped) {
    ASSERT_TRUE(store().save(sampleCache()));
    corruptByte(m_path, readAll(m_path).size() - 3);
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, AlteredTagIsDropped) {
    ASSERT_TRUE(store().save(sampleCache()));
    corruptByte(m_path, 6);
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, BadMagicIsDropped) {
    ASSERT_TRUE(store().save(sampleCache()));
    corruptByte(m_path, 0);
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, FileFromAnotherCodecIsDropped) {
    ASSERT_TRUE(store().save(sampleCache()));
    QByteArray bytes = readAll(m_path);
    bytes[4] = 'Z';
    writeAll(m_path, bytes);
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, TruncatedFileIsDropped) {
    ASSERT_TRUE(store().save(sampleCache()));
    writeAll(m_path, readAll(m_path).left(3));
    EXPECT_FALSE(store().load().valid);
    writeAll(m_path, QByteArray());
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, HandWrittenPlainJsonIsDropped) {
    writeAll(m_path, QByteArrayLiteral("MFE1C{\"tier\":\"pro\",\"trialEnd\":\"\",\"lastSeen\":\"\"}"));
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, SavingAnInvalidCacheRemovesTheFile) {
    ASSERT_TRUE(store().save(sampleCache()));
    ASSERT_TRUE(QFile::exists(m_path));
    ASSERT_TRUE(store().save(EntitlementCache{}));
    EXPECT_FALSE(QFile::exists(m_path));
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, ClearRemovesTheFileAndIsIdempotent) {
    ASSERT_TRUE(store().save(sampleCache()));
    store().clear();
    EXPECT_FALSE(QFile::exists(m_path));
    store().clear();
    EXPECT_FALSE(store().load().valid);
}

TEST_F(EntitlementStoreTest, SaveCreatesMissingFolders) {
    const QString nested = m_dir.filePath(QStringLiteral("a/b/Entitlement.dat"));
    EntitlementStore deep(nested, std::make_unique<ChecksumCodec>());
    ASSERT_TRUE(deep.save(sampleCache()));
    EXPECT_TRUE(deep.load().valid);
}

TEST_F(EntitlementStoreTest, SaveOverwritesThePreviousRecord) {
    ASSERT_TRUE(store().save(sampleCache()));
    EntitlementCache pro = sampleCache();
    pro.tier = Tier::Pro;
    pro.trialEnd = QDateTime();
    ASSERT_TRUE(store().save(pro));
    const EntitlementCache c = store().load();
    EXPECT_EQ(c.tier, Tier::Pro);
    EXPECT_FALSE(c.trialEnd.isValid());
}
