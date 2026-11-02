#include <gtest/gtest.h>

#include <QSignalSpy>

#include "Settings.h"
#include "TestSupport/IsolatedSettings.h"
#include "WhatsNew.h"
#include "WhatsNewController.h"

namespace {
class WhatsNewControllerTest : public ::testing::Test {
    protected:
        void SetUp() override { mftest::clearUserSettings(); }
};
} // namespace

TEST_F(WhatsNewControllerTest, FirstRunRecordsTheVersionAndStaysQuiet) {
    Settings s;
    ASSERT_TRUE(s.lastSeenVersion().isEmpty());
    WhatsNewController c(&s, QStringLiteral("1.7.0"));
    c.start();
    EXPECT_FALSE(c.pending());
    EXPECT_EQ(s.lastSeenVersion(), QStringLiteral("1.7.0"));
}

TEST_F(WhatsNewControllerTest, SameVersionDoesNotShow) {
    Settings s;
    s.setLastSeenVersion(QStringLiteral("1.7.0"));
    WhatsNewController c(&s, QStringLiteral("1.7.0-42.abc"));
    c.start();
    EXPECT_FALSE(c.pending());
    EXPECT_EQ(c.version(), QStringLiteral("1.7.0"));
}

TEST_F(WhatsNewControllerTest, UpdateShowsUntilMarkedSeen) {
    Settings s;
    s.setLastSeenVersion(QStringLiteral("1.6.0"));
    WhatsNewController c(&s, QStringLiteral("1.7.0"));
    QSignalSpy changed(&c, &WhatsNewController::changed);
    c.start();
    EXPECT_TRUE(c.pending());
    EXPECT_EQ(s.lastSeenVersion(), QStringLiteral("1.6.0"));
    EXPECT_EQ(changed.count(), 1);

    c.markSeen();
    EXPECT_FALSE(c.pending());
    EXPECT_EQ(s.lastSeenVersion(), QStringLiteral("1.7.0"));
    EXPECT_EQ(changed.count(), 2);

    WhatsNewController again(&s, QStringLiteral("1.7.0"));
    again.start();
    EXPECT_FALSE(again.pending());
}

TEST_F(WhatsNewControllerTest, MarkSeenWhenNotPendingIsSilent) {
    Settings s;
    s.setLastSeenVersion(QStringLiteral("1.7.0"));
    WhatsNewController c(&s, QStringLiteral("1.7.0"));
    c.start();
    QSignalSpy changed(&c, &WhatsNewController::changed);
    c.markSeen();
    EXPECT_EQ(changed.count(), 0);
}

TEST_F(WhatsNewControllerTest, RequestShowEmits) {
    Settings s;
    WhatsNewController c(&s, QStringLiteral("1.7.0"));
    QSignalSpy shown(&c, &WhatsNewController::showRequested);
    c.requestShow();
    EXPECT_EQ(shown.count(), 1);
}

TEST_F(WhatsNewControllerTest, ExposesTheBundledNotes) {
    Settings s;
    WhatsNewController c(&s);
    EXPECT_EQ(c.text(), mf::whatsNewText());
    EXPECT_EQ(c.body(), mf::whatsNewBody(mf::whatsNewText()));
    EXPECT_FALSE(c.version().isEmpty());
}

TEST_F(WhatsNewControllerTest, WorksWithoutSettings) {
    WhatsNewController c(nullptr, QStringLiteral("1.7.0"));
    c.start();
    EXPECT_FALSE(c.pending());
    c.markSeen();
    SUCCEED();
}
