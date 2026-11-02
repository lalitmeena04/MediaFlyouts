#include <QtTest>

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>

#include "AppTranslator.h"
#include "Capabilities.h"
#include "Settings.h"
#include "TestSupport/IsolatedSettings.h"
#include "TrayController.h"

// Needs QApplication for QMenu, so it is a Qt Test rather than a GoogleTest case.
class TrayControllerQtTest : public QObject {
        Q_OBJECT

    private slots:
        void initTestCase();
        void init();
        void cleanup();
        void menuHasEveryActionInEnglishByDefault();
        void retranslateSwitchesMenuTexts();
        void languageChangedSignalDrivesRetranslate();
        void tooltipIsRetranslatedWithTheMenu();

    private:
        static QList<QAction*> menuActions(TrayController& controller);
        static QStringList texts(const QList<QAction*>& actions);
};

void TrayControllerQtTest::initTestCase() {
    mftest::isolateUserSettings();
}

void TrayControllerQtTest::init() {
    mftest::clearUserSettings();
}

void TrayControllerQtTest::cleanup() {
    // Leave the process in English for the next case.
    AppTranslator reset;
    reset.apply(QStringLiteral("en"));
}

QList<QAction*> TrayControllerQtTest::menuActions(TrayController& controller) {
    auto* tray = controller.findChild<QSystemTrayIcon*>();
    if(!tray || !tray->contextMenu())
        return {};
    QList<QAction*> actions;
    for(QAction* action : tray->contextMenu()->actions())
        if(!action->isSeparator())
            actions << action;
    return actions;
}

QStringList TrayControllerQtTest::texts(const QList<QAction*>& actions) {
    QStringList out;
    for(QAction* action : actions)
        out << action->text();
    return out;
}

void TrayControllerQtTest::menuHasEveryActionInEnglishByDefault() {
    Settings settings;
    TrayController controller(QString(), &settings);
    const QStringList expected = {QStringLiteral("Settings"),          QStringLiteral("Show Media Flyout"),
                                  QStringLiteral("Volume Mixer"),      QStringLiteral("Repository"),
                                  QStringLiteral("Report a Bug"),      QStringLiteral("Open Config Folder"),
                                  QStringLiteral("Restart MediaFlyouts"), QStringLiteral("Quit MediaFlyouts")};
    QCOMPARE(texts(menuActions(controller)), expected);
}

void TrayControllerQtTest::retranslateSwitchesMenuTexts() {
    Settings settings;
    TrayController controller(QString(), &settings);

    AppTranslator translator;
    translator.apply(QStringLiteral("de"));
    controller.retranslate();
    QStringList german = texts(menuActions(controller));
    QCOMPARE(german.size(), 8);
    QCOMPARE(german.first(), QStringLiteral("Einstellungen"));
    QCOMPARE(german.last(), QStringLiteral("MediaFlyouts beenden"));

    translator.apply(QStringLiteral("en"));
    controller.retranslate();
    QCOMPARE(texts(menuActions(controller)).first(), QStringLiteral("Settings"));
}

void TrayControllerQtTest::languageChangedSignalDrivesRetranslate() {
    // Same wiring as MainControllerClass: the translator's signal refreshes the menu.
    Settings settings;
    TrayController controller(QString(), &settings);
    AppTranslator translator;
    connect(&translator, &AppTranslator::languageChanged, &controller, &TrayController::retranslate);

    translator.apply(QStringLiteral("fr"));
    QCOMPARE(texts(menuActions(controller)).first(), QStringLiteral("Paramètres"));

    translator.apply(QStringLiteral("es"));
    QCOMPARE(texts(menuActions(controller)).first(), QStringLiteral("Ajustes"));
}

void TrayControllerQtTest::tooltipIsRetranslatedWithTheMenu() {
    Settings settings;
    TrayController controller(QString(), &settings);
    Capabilities caps;
    caps.setAudioAvailable(false);
    controller.trackCapabilities(&caps);
    auto* tray = controller.findChild<QSystemTrayIcon*>();
    QVERIFY(tray);
    QCOMPARE(tray->toolTip(), QStringLiteral("MediaFlyouts\nUnavailable: Audio devices"));

    AppTranslator translator;
    translator.apply(QStringLiteral("de"));
    controller.retranslate();
    QCOMPARE(tray->toolTip(), QStringLiteral("MediaFlyouts\nNicht verfügbar: Audiogeräte"));
}

QTEST_MAIN(TrayControllerQtTest)
#include "TrayControllerQtTest.moc"
