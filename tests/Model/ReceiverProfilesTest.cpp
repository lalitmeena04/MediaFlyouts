#include <gtest/gtest.h>

#include "ReceiverProfiles.h"

namespace {

const mf::ReceiverProfile* findProfile(const QVector<mf::ReceiverProfile>& profiles, quint16 vendorId,
                                       quint16 productId) {
    for(const mf::ReceiverProfile& profile : profiles)
        if(profile.vendorId == vendorId && profile.productId == productId)
            return &profile;
    return nullptr;
}

} // namespace

TEST(ReceiverProfilesTest, ShipsTheKnownReceiversAsAStartingPoint) {
    const QVector<mf::ReceiverProfile> defaults = mf::defaultReceiverProfiles();

    const mf::ReceiverProfile* aula = findProfile(defaults, 0x3554, 0xFA09);
    ASSERT_NE(aula, nullptr);
    EXPECT_EQ(aula->name, QStringLiteral("AULA wireless keyboard"));
    EXPECT_EQ(aula->wiredVendorId, 0x258A);
    EXPECT_EQ(aula->wiredProductId, 0x010C);

    const mf::ReceiverProfile* g435 = findProfile(defaults, 0x046D, 0x0ADF);
    ASSERT_NE(g435, nullptr);
    EXPECT_EQ(g435->wiredVendorId, 0);
}

TEST(ReceiverProfilesTest, ReadsHexAndNumericIds) {
    const QByteArray json = R"({"receivers":[
        {"vid":"1B2C","pid":"0x00FF","name":"Hex Receiver","category":"mouse"},
        {"vid":1234,"pid":10,"name":"Numeric Receiver"}
    ]})";

    QStringList errors;
    const QVector<mf::ReceiverProfile> profiles = mf::parseReceiverProfiles(json, &errors);

    ASSERT_EQ(profiles.size(), 2);
    EXPECT_EQ(profiles[0].vendorId, 0x1B2C);
    EXPECT_EQ(profiles[0].productId, 0x00FF);
    EXPECT_EQ(profiles[0].category, QStringLiteral("mouse"));
    EXPECT_EQ(profiles[1].vendorId, 1234);
    EXPECT_TRUE(profiles[1].category.isEmpty());
    EXPECT_TRUE(errors.isEmpty());
}

TEST(ReceiverProfilesTest, AcceptsABareArray) {
    const QByteArray json = R"([{"vid":"1B2C","pid":"00FF","name":"Bare"}])";
    EXPECT_EQ(mf::parseReceiverProfiles(json).size(), 1);
}

TEST(ReceiverProfilesTest, SkipsMalformedEntriesAndExplainsWhy) {
    const QByteArray json = R"({"receivers":[
        {"pid":"00FF","name":"No vendor"},
        {"vid":"1B2C","name":"No product"},
        {"vid":"1B2C","pid":"00FF"},
        {"vid":"ZZZZ","pid":"00FF","name":"Bad hex"},
        "not an object",
        {"vid":"1B2C","pid":"00FF","name":"Good"}
    ]})";

    QStringList errors;
    const QVector<mf::ReceiverProfile> profiles = mf::parseReceiverProfiles(json, &errors);

    ASSERT_EQ(profiles.size(), 1);
    EXPECT_EQ(profiles[0].name, QStringLiteral("Good"));
    EXPECT_EQ(errors.size(), 5);
}

// A twin with only one id would match every device sitting at id zero.
TEST(ReceiverProfilesTest, RejectsAHalfDeclaredWiredTwin) {
    const QByteArray json = R"([{"vid":"1B2C","pid":"00FF","name":"Half","wiredVid":"258A"}])";

    QStringList errors;
    EXPECT_TRUE(mf::parseReceiverProfiles(json, &errors).isEmpty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains(QStringLiteral("wiredVid")));
}

TEST(ReceiverProfilesTest, ReportsUnreadableJson) {
    QStringList errors;
    EXPECT_TRUE(mf::parseReceiverProfiles("{ not json", &errors).isEmpty());
    ASSERT_EQ(errors.size(), 1);
    EXPECT_TRUE(errors.first().contains(QStringLiteral("invalid JSON")));
}

TEST(ReceiverProfilesTest, EmptyInputIsNotAnError) {
    QStringList errors;
    EXPECT_TRUE(mf::parseReceiverProfiles(QByteArray(), &errors).isEmpty());
    EXPECT_TRUE(errors.isEmpty());
}

TEST(ReceiverProfilesTest, UserProfileReplacesTheBuiltInOfTheSameDevice) {
    const QVector<mf::ReceiverProfile> defaults = mf::defaultReceiverProfiles();
    const QVector<mf::ReceiverProfile> overrides =
        mf::parseReceiverProfiles(R"([{"vid":"3554","pid":"FA09","name":"My keyboard","wiredVid":"1111",
                                       "wiredPid":"2222"}])");

    const QVector<mf::ReceiverProfile> merged = mf::mergeReceiverProfiles(defaults, overrides);

    EXPECT_EQ(merged.size(), defaults.size());
    const mf::ReceiverProfile* aula = findProfile(merged, 0x3554, 0xFA09);
    ASSERT_NE(aula, nullptr);
    EXPECT_EQ(aula->name, QStringLiteral("My keyboard"));
    EXPECT_EQ(aula->wiredVendorId, 0x1111);
}

TEST(ReceiverProfilesTest, UnknownUserProfileIsAppended) {
    const QVector<mf::ReceiverProfile> defaults = mf::defaultReceiverProfiles();
    const QVector<mf::ReceiverProfile> overrides =
        mf::parseReceiverProfiles(R"([{"vid":"1B2C","pid":"00FF","name":"New dongle"}])");

    const QVector<mf::ReceiverProfile> merged = mf::mergeReceiverProfiles(defaults, overrides);

    EXPECT_EQ(merged.size(), defaults.size() + 1);
    EXPECT_NE(findProfile(merged, 0x1B2C, 0x00FF), nullptr);
    EXPECT_NE(findProfile(merged, 0x046D, 0x0ADF), nullptr);
}
