#include <QtTest>

#include "FlyoutPositioner.h"

using namespace mf;

class FlyoutPositionerQtTest : public QObject {
        Q_OBJECT

    private slots:
        void preservesSize();
        void corners_data();
        void corners();
        void honorsWorkAreaOrigin();
};

void FlyoutPositionerQtTest::preservesSize() {
    const QRect r = computeFlyoutRect(QRect(0, 0, 1920, 1080), QSize(360, 120), FlyoutPosition::TopLeft, 24);
    QCOMPARE(r.size(), QSize(360, 120));
}

void FlyoutPositionerQtTest::corners_data() {
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("x");
    QTest::addColumn<int>("y");

    QTest::newRow("top-left") << int(FlyoutPosition::TopLeft) << 24 << 24;
    QTest::newRow("top-center") << int(FlyoutPosition::TopCenter) << (1920 - 360) / 2 << 24;
    QTest::newRow("top-right") << int(FlyoutPosition::TopRight) << 1920 - 360 - 24 << 24;
    QTest::newRow("bottom-left") << int(FlyoutPosition::BottomLeft) << 24 << 1080 - 120 - 24;
    QTest::newRow("bottom-center") << int(FlyoutPosition::BottomCenter) << (1920 - 360) / 2 << 1080 - 120 - 24;
    QTest::newRow("bottom-right") << int(FlyoutPosition::BottomRight) << 1920 - 360 - 24 << 1080 - 120 - 24;
}

void FlyoutPositionerQtTest::corners() {
    QFETCH(int, position);
    QFETCH(int, x);
    QFETCH(int, y);
    const QRect r = computeFlyoutRect(QRect(0, 0, 1920, 1080), QSize(360, 120), FlyoutPosition(position), 24);
    QCOMPARE(r.x(), x);
    QCOMPARE(r.y(), y);
}

void FlyoutPositionerQtTest::honorsWorkAreaOrigin() {
    const QRect r = computeFlyoutRect(QRect(100, 200, 1920, 1080), QSize(360, 120), FlyoutPosition::TopLeft, 24);
    QCOMPARE(r.x(), 124);
    QCOMPARE(r.y(), 224);
}

QTEST_GUILESS_MAIN(FlyoutPositionerQtTest)
#include "FlyoutPositionerQtTest.moc"
