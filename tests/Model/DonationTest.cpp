#include <gtest/gtest.h>

#include <QString>

#include "Donation.h"

using mf::DonationConfig;
using mf::DonationLayout;
using mf::InstallKind;

namespace {
DonationConfig full() {
    return mf::parseDonationConfig(QStringLiteral(" https://example.org/donate/ "), QStringLiteral(" dev@upi "),
                                   QStringLiteral(" Dev Name "), QStringLiteral("Some Bank"),
                                   QStringLiteral("1234567890"), QStringLiteral("abcd0001234"),
                                   QStringLiteral("Dev Name"));
}
} // namespace

TEST(DonationTest, ParsesAndTrimsEveryField) {
    const DonationConfig c = full();
    EXPECT_EQ(c.pageUrl, QStringLiteral("https://example.org/donate/"));
    EXPECT_EQ(c.upiId, QStringLiteral("dev@upi"));
    EXPECT_EQ(c.payeeName, QStringLiteral("Dev Name"));
    EXPECT_EQ(c.bankName, QStringLiteral("Some Bank"));
    EXPECT_EQ(c.bankAccount, QStringLiteral("1234567890"));
    EXPECT_EQ(c.bankIfsc, QStringLiteral("ABCD0001234"));
    EXPECT_EQ(c.bankHolder, QStringLiteral("Dev Name"));
    EXPECT_TRUE(c.hasPage());
    EXPECT_TRUE(c.hasUpi());
    EXPECT_TRUE(c.hasBank());
    EXPECT_TRUE(c.any());
}

TEST(DonationTest, EmptyConfigOffersNothing) {
    const DonationConfig c = mf::parseDonationConfig({}, {}, {}, {}, {}, {}, {});
    EXPECT_FALSE(c.hasPage());
    EXPECT_FALSE(c.hasUpi());
    EXPECT_FALSE(c.hasBank());
    EXPECT_FALSE(c.any());
}

TEST(DonationTest, RejectsNonHttpPageAndMalformedUpiId) {
    const DonationConfig c = mf::parseDonationConfig(QStringLiteral("file:///C:/x"), QStringLiteral("noatsign"), {},
                                                      {}, {}, {}, {});
    EXPECT_FALSE(c.hasPage());
    EXPECT_FALSE(c.hasUpi());
    EXPECT_TRUE(mf::parseDonationConfig(QStringLiteral("http://example.org"), {}, {}, {}, {}, {}, {}).hasPage());
}

TEST(DonationTest, BankNeedsAccountAndIfsc) {
    EXPECT_FALSE(mf::parseDonationConfig({}, {}, {}, {}, QStringLiteral("123"), {}, {}).hasBank());
    EXPECT_FALSE(mf::parseDonationConfig({}, {}, {}, {}, {}, QStringLiteral("ABCD0001234"), {}).hasBank());
    EXPECT_TRUE(mf::parseDonationConfig({}, {}, {}, {}, QStringLiteral("123"), QStringLiteral("ABCD0001234"), {})
                    .hasBank());
}

TEST(DonationTest, StoreBuildsLinkOutEverythingElseIsDirect) {
    EXPECT_EQ(mf::donationLayoutFor(InstallKind::Store), DonationLayout::StoreLink);
    EXPECT_EQ(mf::donationLayoutFor(InstallKind::Installer), DonationLayout::Direct);
    EXPECT_EQ(mf::donationLayoutFor(InstallKind::Portable), DonationLayout::Direct);
    EXPECT_EQ(mf::donationLayoutFor(InstallKind::Unknown), DonationLayout::Direct);
}

TEST(DonationTest, UpiUriKeepsTheAtSignAndEncodesTheRest) {
    EXPECT_EQ(mf::upiPaymentUri(QStringLiteral("dev@upi"), QStringLiteral("Dev Name"), QStringLiteral("Thanks & more")),
              QStringLiteral("upi://pay?pa=dev@upi&pn=Dev%20Name&cu=INR&tn=Thanks%20%26%20more"));
    EXPECT_EQ(mf::upiPaymentUri(QStringLiteral(" dev@upi "), QString()), QStringLiteral("upi://pay?pa=dev@upi&cu=INR"));
    EXPECT_TRUE(mf::upiPaymentUri(QString(), QStringLiteral("Dev")).isEmpty());
}

TEST(DonationTest, CompiledConfigIsReadable) {
    const DonationConfig c = mf::donationConfig();
    EXPECT_EQ(mf::donationConfig().pageUrl, c.pageUrl);
    if(c.hasPage())
        EXPECT_TRUE(c.pageUrl.startsWith(QStringLiteral("http")));
}
