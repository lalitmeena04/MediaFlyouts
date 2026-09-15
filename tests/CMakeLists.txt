# --- Cross-platform app classes the tests exercise (compiled into a helper lib) ---
qt_add_library(mediaflyout_testlib STATIC
    ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.h
    ${CMAKE_SOURCE_DIR}/Controllers/ThemeController.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/ThemeController.h
    ${CMAKE_SOURCE_DIR}/Controllers/UpdateChecker.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/UpdateChecker.h
    ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.cpp
    ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.h
    ${CMAKE_SOURCE_DIR}/Model/Settings.cpp
    ${CMAKE_SOURCE_DIR}/Model/Settings.h
)
target_link_libraries(mediaflyout_testlib PUBLIC
    mediaflyout_core
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
)
target_include_directories(mediaflyout_testlib PUBLIC
    ${CMAKE_SOURCE_DIR}/Model
    ${CMAKE_SOURCE_DIR}/Controllers
)

# --- GoogleTest / GMock: fetched, used for pure-logic and mocking ---
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)
# Match Qt dynamically-linked CRT on MSVC.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# One GoogleTest executable aggregating every per-class test file.
add_executable(mediaflyout_gtest
    Controllers/NowPlayingTest.cpp
    Controllers/ThemeControllerTest.cpp
    Controllers/UpdateCheckerTest.cpp
    gtest_main.cpp
    Model/AlbumArtProviderTest.cpp
    Model/BatteryAlertPolicyTest.cpp
    Model/BatteryUtilsTest.cpp
    Model/CapabilitiesTest.cpp
    Model/DualSenseProtocolTest.cpp
    Model/FastPairProtocolTest.cpp
    Model/FftTest.cpp
    Model/FlyoutPositionerTest.cpp
    Model/HidppProtocolTest.cpp
    Model/PlaybackControllerTest.cpp
    Model/PowerAlertPolicyTest.cpp
    Model/RazerProtocolTest.cpp
    Model/ReceiverProfilesTest.cpp
    Model/SessionSelectionTest.cpp
    Model/SettingsTest.cpp
    Model/TaskbarLayoutTest.cpp
    Model/TrackInfoTest.cpp
    Model/WiredPeripheralRulesTest.cpp
    Views/QmlContractTest.cpp
)
target_link_libraries(mediaflyout_gtest PRIVATE
    mediaflyout_testlib
    Qt6::Test
    GTest::gtest
    GTest::gmock
)
target_compile_definitions(mediaflyout_gtest PRIVATE MEDIAFLYOUTS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME gtest_all COMMAND mediaflyout_gtest)

# Windows-only audio/DWM controllers get default-state smoke tests
if(WIN32)
    target_sources(mediaflyout_gtest PRIVATE
        ${CMAKE_SOURCE_DIR}/Controllers/AudioCapture.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/BluetoothClassicBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/BluetoothLeBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/FastPairBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/GameControllerBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/HidCollections.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/LogitechBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/PlayStationControllerBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/RazerBatteryProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/WiredPeripheralProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryProviders/WirelessReceiverProvider.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BatteryStatusController.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/BluetoothConnectionWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/HidChangeWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/MonitorMapper.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/PeripheralAlerts.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/PowerStatusController.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/TaskbarInfo.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/VolumeMixer.cpp
        ${CMAKE_SOURCE_DIR}/Controllers/WindowEffects.cpp
        Controllers/BatteryStatusControllerTest.cpp
        Controllers/HidCollectionsTest.cpp
        Controllers/MonitorMapperTest.cpp
        Controllers/PeripheralAlertsTest.cpp
        Controllers/Win32ControllersTest.cpp
        Controllers/WirelessReceiverProviderTest.cpp
        Packaging/MsixManifestTest.cpp
    )
    target_compile_definitions(mediaflyout_gtest PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(mediaflyout_gtest PRIVATE
        WindowsApp.lib
        dwmapi.lib
        shell32.lib
        gdi32.lib
        user32.lib
        cfgmgr32.lib
        Bthprops.lib
        hid.lib
        setupapi.lib
        ws2_32.lib
        ole32.lib
        oleaut32.lib
        uiautomationcore.lib
    )
endif()

# --- Qt Test: one executable per class (each file has its own QTEST_GUILESS_MAIN) ---
set(qt_test_sources
    Controllers/NowPlayingQtTest.cpp
    Model/AlbumArtProviderQtTest.cpp
    Model/FlyoutPositionerQtTest.cpp
    Model/PlaybackControllerQtTest.cpp
    Model/SettingsQtTest.cpp
    Model/TrackInfoQtTest.cpp
)
set(qt_test_names "")
foreach(src IN LISTS qt_test_sources)
    get_filename_component(name ${src} NAME_WE)
    qt_add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE Qt6::Test mediaflyout_testlib)
    add_test(NAME ${name} COMMAND ${name})
    list(APPEND qt_test_names ${name})
endforeach()

# Qt DLLs live in bin dir. Prepend it to PATH for test executables.
set_tests_properties(gtest_all ${qt_test_names} PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

