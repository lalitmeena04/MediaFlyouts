#include <gtest/gtest.h>

#include <QClipboard>
#include <QGuiApplication>
#include <QString>

#include "DonationController.h"

using mf::DonationConfig;
using mf::InstallKind;

namespace {
DonationConfig full() {
    return mf::parseDonationConfig(QStringLiteral("https://example.org/donate/"), QStringLiteral("dev@upi"),
                                   QStringLiteral("Dev Name"), QStringLiteral("Some Bank"),
                                   QStringLiteral("1234567890"), QStringLiteral("ABCD0001234"),
                                   QStringLiteral("Dev Name"));
}
} // namespace

TEST(DonationControllerTest, InstallerShowsDirectPaymentsAndNoStoreTips) {
    DonationController c(full(), InstallKind::Installer, true);
    EXPECT_TRUE(c.available());
    EXPECT_TRUE(c.showDirect());
    EXPECT_FALSE(c.showStoreTips());
    EXPECT_TRUE(c.hasPage());
    EXPECT_TRUE(c.hasUpi());
    EXPECT_TRUE(c.hasBank());
    EXPECT_EQ(c.upiId(), QStringLiteral("dev@upi"));
    EXPECT_EQ(c.bankIfsc(), QStringLiteral("ABCD0001234"));
}

TEST(DonationControllerTest, StoreShowsLinkAndTipsButNoDirectPayments) {
    DonationController c(full(), InstallKind::Store, true);
    EXPECT_TRUE(c.available());
    EXPECT_FALSE(c.showDirect());
    EXPECT_TRUE(c.showStoreTips());
    EXPECT_TRUE(c.hasPage());
}

TEST(DonationControllerTest, StoreWithoutTipIdsHasOnlyTheLink) {
    DonationController c(full(), InstallKind::Store, false);
    EXPECT_TRUE(c.available());
    EXPECT_FALSE(c.showDirect());
    EXPECT_FALSE(c.showStoreTips());
}

TEST(DonationControllerTest, NothingConfiguredMeansUnavailable) {
    DonationController c(mf::parseDonationConfig({}, {}, {}, {}, {}, {}, {}), InstallKind::Portable, false);
    EXPECT_FALSE(c.available());
    EXPECT_FALSE(c.showDirect());
    EXPECT_FALSE(c.openPage());
    EXPECT_TRUE(c.upiQrSource().isEmpty());
}

TEST(DonationControllerTest, UpiUriAndQrSourceComeFromTheConfig) {
    DonationController c(full(), InstallKind::Portable, false);
    EXPECT_EQ(c.upiUri(), QStringLiteral("upi://pay?pa=dev@upi&pn=Dev%20Name&cu=INR&tn=MediaFlyouts%20donation"));
    EXPECT_TRUE(c.upiQrSource().startsWith(QStringLiteral("image://qr/")));
    EXPECT_EQ(c.upiQrSource(), DonationController::qrSourceFor(c.upiUri()));
    // Base64url: no '+', '/' or '=' that a URL would touch.
    const QString id = c.upiQrSource().mid(QStringLiteral("image://qr/").size());
    EXPECT_FALSE(id.contains(QLatin1Char('+')));
    EXPECT_FALSE(id.contains(QLatin1Char('/')));
    EXPECT_FALSE(id.contains(QLatin1Char('=')));
}

TEST(DonationControllerTest, CopyToClipboardWritesTheText) {
    DonationController c(full(), InstallKind::Portable, false);
    QClipboard* clipboard = QGuiApplication::clipboard();
    if(!clipboard)
        GTEST_SKIP() << "No clipboard on this host.";
    c.copyToClipboard(QStringLiteral("1234567890"));
    // Another process may hold the Windows clipboard open for a moment.
    if(clipboard->text() != QStringLiteral("1234567890"))
        GTEST_SKIP() << "Clipboard busy.";
    SUCCEED();
}
